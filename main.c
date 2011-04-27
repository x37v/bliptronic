#include "midi_usb.h"
#include "avr/io.h"

void fallthrough_callback(MidiDevice * device, uint16_t cnt, uint8_t byte0, uint8_t byte1, uint8_t byte2){
   midi_send_data(device, cnt, byte0, byte1, byte2);
}


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
