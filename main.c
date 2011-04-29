#include "midi_usb.h"
#include <avr/io.h>
#include <util/delay.h>

void fallthrough_callback(MidiDevice * device, uint16_t cnt, uint8_t byte0, uint8_t byte1, uint8_t byte2){
   midi_send_data(device, cnt, byte0, byte1, byte2);
}

/*
 * as reference
 * http://www.straytechnologies.com/bliptronic-5000-hacking-full-pin-out-to-the-64-button-pcb/
 *
 * outputs:
 *
 * button power:
 * b5 u7 reset
 * b4 u7 clock
 *
 * led power [row]:
 * b3 u5 clock
 * b2 u5 reset
 *
 * led grounds [column]:
 * b1 u3 SH_CP [data reg, serial in, pos edge]
 * b0 u3 ST_CP [storage reg, parallel out, pos edge]
 *
 * e0 shared serial out
 *
 * button inputs:
 * porta
 */

#define SH_CP PB1
#define ST_CP PB0

#define LED_POWER_CLK PB3
#define LED_POWER_RST PB2

volatile uint8_t cur_led_row;

//set the serial out value to the LS bit of v
void setSerial(uint8_t v) {
   //set pin e0
   PORTE = (PINE & ~0x1) | (v & 0x1);
   _delay_us(10);
}

void shiftLedCol(uint8_t col) {
   uint8_t i;
   PORTB &= ~_BV(ST_CP);
   for(i = 0; i < 8; i++) {
      PORTB &= ~_BV(SH_CP);
      setSerial(col >> i);
      PORTB |= _BV(SH_CP);
   }
   PORTB |= _BV(ST_CP);
}


int main(void) {
   uint8_t i;
   MidiDevice usb_midi;
   midi_usb_init(&usb_midi);

   //e0 serial output
   DDRE |= _BV(PE0);

	//porta is all inputs with pullups
	DDRA = 0;
	PORTA = 0xFF;

   //portb all outputs
   DDRB = 0xFF;

   PORTB |= _BV(LED_POWER_RST);

   midi_register_fallthrough_callback(&usb_midi, fallthrough_callback);

   PORTB &= ~_BV(LED_POWER_CLK);
   setSerial(0);
   PORTB |= _BV(LED_POWER_CLK);

   PORTB &= ~_BV(LED_POWER_CLK);
   setSerial(1);
   PORTB |= _BV(LED_POWER_CLK);

   for(i = 1; i < 7; i++) {
      PORTB &= ~_BV(LED_POWER_CLK);
      setSerial(0);
      PORTB |= _BV(LED_POWER_CLK);
   }

   while(1){
      shiftLedCol(0xf0);
      midi_device_process(&usb_midi);
   }

   return 0;
}
