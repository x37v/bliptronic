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
   col = ~col;
   PORTB &= ~_BV(ST_CP);
   for(i = 0; i < 8; i++) {
      PORTB &= ~_BV(SH_CP);
      setSerial(col >> i);
      PORTB |= _BV(SH_CP);
   }
   PORTB |= _BV(ST_CP);
}

void updateLedRow() {
   PORTB &= ~_BV(LED_POWER_CLK);
   cur_led_row++;
   if(cur_led_row > 8) {
      cur_led_row = 0;
      setSerial(1);
   } else {
      setSerial(0);
   }
   PORTB |= _BV(LED_POWER_CLK);
}

void init(MidiDevice * usb_midi, uint8_t *leds) {
   uint8_t i;

   midi_usb_init(usb_midi);

   //e0 serial output
   DDRE |= _BV(PE0);

	//porta is all inputs with pullups
	DDRA = 0;
	PORTA = 0xFF;

   //portb all outputs
   DDRB = 0xFF;

   PORTB |= _BV(LED_POWER_RST);

   midi_register_fallthrough_callback(&usb_midi, fallthrough_callback);

   for(i = 1; i < 8; i++) {
      PORTB &= ~_BV(LED_POWER_CLK);
      setSerial(0);
      PORTB |= _BV(LED_POWER_CLK);
   }

   PORTB &= ~_BV(LED_POWER_CLK);
   setSerial(1);
   PORTB |= _BV(LED_POWER_CLK);

   cur_led_row = 0;

   for(i = 0; i < 8; i++)
      leds[i] = 0;
}

void drawLeds(uint8_t * leds) {
   updateLedRow();
   shiftLedCol(leds[cur_led_row]);
}

int main(void) {
   uint8_t i = 0;
   MidiDevice usb_midi;
   uint8_t leds[8];

   init(&usb_midi, leds);

   //make the leds into an 'A'
   leds[0] = 0x18;
   leds[1] = 0x24;
   leds[2] = 0x42;
   for(i = 3; i < 8; i++)
      leds[i] = 0x81;
   leds[4] = 0xFF;

   while(1){
      drawLeds(leds);
      _delay_us(100);
      midi_device_process(&usb_midi);
      shiftLedCol(0x00);
      _delay_us(10);
   }

   return 0;
}
