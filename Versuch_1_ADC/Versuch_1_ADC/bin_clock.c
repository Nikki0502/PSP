#include "bin_clock.h"

#include <avr/interrupt.h>
#include <avr/io.h>

//! Global variables
uint8_t stunden;
uint8_t minuten;
uint8_t sekunden;
uint16_t millisek;

/*!
 * \return The milliseconds counter of the current time.
 */
uint16_t getTimeMilliseconds() {
    return millisek;
}

/*!
 * \return The seconds counter of the current time.
 */
uint8_t getTimeSeconds() {
    return sekunden;
}

/*!
 * \return The minutes counter of the current time.
 */
uint8_t getTimeMinutes() {
    return minuten;
}

/*!
 * \return The hour counter of the current time.
 */
uint8_t getTimeHours() {
    return stunden;
}

/*!
 *  Initializes the binary clock (ISR and global variables)
 */
void initClock(void) {
    // Set timer mode to CTC
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);

    // Set prescaler to 1024
    TCCR0B |= (1 << CS02) | (1 << CS00);
    TCCR0B &= ~(1 << CS01);

    // Set compare register to 195 -> match every 10ms
    OCR0A = 195;

	// Init variables
	stunden=12;
	minuten=59;
	sekunden=45;
	millisek=000;

    // Enable timer and global interrupts
    TIMSK0 |= (1 << OCIE0A);
    sei();
}

/*!
 *  Updates the global variables to get a valid 12h-time
 */
void updateClock(void){
  if(millisek==1000){//groeßer gleich stuff fuer eventuelle Fehler idk ob notwendig
	  millisek=0;
	  sekunden=sekunden+1;
  }
  if(sekunden==60){
	  sekunden=0;
	  minuten=minuten+1;
  }
  if(minuten==60){
	  minuten=0;
	  stunden=stunden+1;
  }
  if(stunden==13){
	  stunden=1;
  }
}

/*!
 *  ISR to increase millisecond-counter of the clock
 */
ISR(TIMER0_COMPA_vect) {
  millisek=millisek+10;
  updateClock();
}
