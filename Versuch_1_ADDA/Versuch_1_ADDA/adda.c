#include "adda.h"
#include "os_input.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

void initLEDandR2R(void){
	DDRD &= 0b00000000;//DIP Switch Input
	PORTD |= 0b11111111;//Pull-Up for DIP Input
	DDRA |= 0b11111111;//LEDS as Output
	DDRB |= 0b11111111;//R-2R-Network as Output

}


/*Digital to Analog R-2R-Network*/
void manuell(void){
	while(true){ //FRAGE ZU kontinuierlriche uebertragung!!!!!!!!!!!!!!!!!!!
		uint8_t pinstate = (~PIND &0b11111111);//current DIP Switches
		PORTA = ~pinstate;//matching Leds Output
		PORTB = pinstate;//matching R-2R Output
	}
}
/*Tracking-Wandler*/
void tracking(void){
	while(true){
		os_waitForInput(); // C1 pressed
		uint8_t ref = 0b10000000;//	"Variable ref festgehalten, welche zu Beginn mit einem beliebigen Wert initialisiert werden kann." so i choose the middle
		PORTB |= ref;
		PORTA = ~ref; // display changes with LEDS
		_delay_ms(50);// wait for change to affect the comparator
		bool begining = ((PINC&0b0000001) == 1); // true if Uref>Umess false otherwise
		bool current = begining;
		while(begining == current){
			if(current== true){
				ref = ref - 1 ;
			}
			else{
				ref = ref + 1;
			}
			PORTB |= ref;
			PORTB &= ref;
			PORTA = ~ref; // display changes with LEDS
			// display changes with LEDS
			_delay_ms(50);// wait for change to affect the comparator
			current = ((PINC&0b0000001) == 1);
			if(ref==0b0000000 || ref ==0b11111111){break;}
		}
	}
	
}
/*SA-Wandler*/
void sar(void){
	while (true){
		os_waitForInput();//Begins when C1 is pressed;
		uint8_t ref = 0b00000000;
		PORTB &= ref;// ref U anlegen am R-2R
		PORTA &= ref;// display changes with LEDS
		for(int i = 7; i>=0;i--){
			ref |=(1<<i);//changes Uref by setting MSB to 1
			PORTB |= ref;
			PORTA = ~ref;// display changes with LEDS
			_delay_ms(50);// wait for change to affect the comparator
			if((PINC&0b0000001) == 1){ // testing if Uref is larger than Umess
				ref &=~(1<<i);
				PORTB &= ref;
				PORTA = ~ref;// display changes with LEDS
			}
		}
	}
}