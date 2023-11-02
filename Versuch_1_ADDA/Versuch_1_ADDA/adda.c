#include "adda.h"
#include "os_input.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>

/*Digital to Analog R-2R-Network*/
void manuell(void){
	DDRD &= 0b00000000;//DIP Switch Input
	PORTD |= 0b11111111;//Pull-Up for DIP Input
	DDRA |= 0b11111111;//LEDS Output
	DDRB |= 0b11111111;//R-2R-Network Output
	uint8_t pinstate = (~PIND);//current DIP Switches 
	PORTA |= pinstate;//matching Leds Output
	PORTB |= pinstate;//matching R-2R Output
}
/*Tracking-Wandler*/ 
void tracking(void){
	manuell();
	os_waitForInput();
}
/*SA-Wandler*/
void sar(void){
	manuell();
	os_waitForInput();
}