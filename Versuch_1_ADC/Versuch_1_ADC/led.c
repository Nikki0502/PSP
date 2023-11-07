#include "led.h"

#include <avr/io.h>

uint16_t activateLedMask = 0xFFFF;

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {// solln hier trozdem alle als output sein?ß? steht ja oben FRAGESTUNDE
  if(activateLedMask == 0xFFFF){
	   DDRA |= 0b11111111;
	   PORTA &=0b00000000;
  }
  else{
	  DDRA |= 0b11111110;
	  PORTA &=0b00000001;
  }
  DDRD |= 0xFF;
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {
  uint8_t valueA;
  uint8_t valueD;
  // uses all LEDS
  if(activateLedMask == 0xFFFF){
	valueA=(value & 0b0000000011111111);
	valueD=(value & 0b1111111100000000)>>8;  
  }
  // doesnt use A0 LED
  else{
	valueA=(value & 0b0000000001111111)<<1;
	valueD=(value & 0b0111111110000000)>>7;  
  }
  PORTA = ~valueA;
  PORTD = ~valueD;
}
