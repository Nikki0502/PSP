#include "adda.h"
#include "os_input.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>

/*Digital to Analog R-2R-Network*/
void manuell(void){
	DDRD &= 0b00000000;//DIP Switch Input
	PORTD |= 0b11111111;//Pull-Up for DIP Input
	DDRA |= 0b11111111;//LEDS as Output
	DDRB |= 0b11111111;//R-2R-Network as Output
	uint8_t pinstate = (~PIND);//current DIP Switches 
	PORTA |= pinstate;//matching Leds Output
	PORTB |= pinstate;//matching R-2R Output
}
/*Tracking-Wandler*/ 
void tracking(void){
	os_waitForInput();
}
/*SA-Wandler*/
void sar(void){
	while (true){
		os_waitForInput();//Begins when C1 is pressed;
		uint8_t ref = 0b00000000;
		PORTB &= ref;// ref U anlegen am R-2R
		PORTA &= ref;// display changes wiht LEDS
		for(int i = 7; i>=0;i--){
			ref |=(1<<i);//changes Uref by setting MSB to 1
			PORTB |= ref;
			PORTA |= ref;// display changes wiht LEDS
			_delay_ms(50);// wait for change to affect the comparator 
			if(PINC0 == 1){ // testing if Uref is larger than Umess 
				ref &=~(1<<i);
				PORTB &= ref;
				PORTA &= ref;// display changes wiht LEDS
			}
		}
	}
}