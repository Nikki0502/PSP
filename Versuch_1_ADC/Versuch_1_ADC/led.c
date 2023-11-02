#include "led.h"

#include <avr/io.h>

uint16_t activateLedMask = 0xFFFF;

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {
  DDRA |=0b11111111;
  DDRD |=0b11111111;
  PORTA &=0b00000000;
  PORTD &=0b00000000;
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {
  uint8_t valueA=(value & 0b0000000011111111);
  uint8_t valueD=(value & 0b1111111100000000)>>8;
  PORTA |= valueA;
  PORTD |= valueD;
}
