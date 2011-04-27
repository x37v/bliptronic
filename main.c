#include "midi_usb.h"
#include "avr/io.h"

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
 * led power [columns]:
 * b3 u5 clock
 * b2 u5 reset
 *
 * led grounds [rows]:
 * b1 u3 SH_CP
 * b0 u3 ST_CP
 *
 * e0 shared serial out
 *
 * button inputs:
 * porta
 */


int main(void) {
   MidiDevice usb_midi;
   midi_usb_init(&usb_midi);

   //e0 serial output
   DDRE |= _BV(PE0);

	//porta is all inputs with pullups
	DDRA = 0;
	PORTA = 0xFF;

   //portb all outputs
   DDRB = 0xFF;

   midi_register_fallthrough_callback(&usb_midi, fallthrough_callback);

   while(1){
      midi_device_process(&usb_midi);
   }

   return 0;
}
