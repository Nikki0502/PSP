/*! \file
 *  \brief Functions to draw premade things on the LED Panel
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2019
 */
#include "led_paneldriver.h"

#include "defines.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>


uint8_t frambuffer[2][16][32];


//! \brief Enable compare match interrupts for Timer 1
void panel_startTimer() {
    sbi(TIMSK1, OCIE1A);
}

//! \brief Disable compare match interrupts for Timer 1
void panel_stopTimer() {
    cbi(TIMSK1, OCIE1A);
}

//!!!NOCH AN GEEINGNETER STELLE AUFRUFEN!!!

//! \brief Initialization function of Timer 1
void panel_initTimer() {
    // Configuration TCCR1B register
    sbi(TCCR1B, WGM12); // Clear on timer compare match
    sbi(TCCR1B, CS12);  // Prescaler 256  1
    cbi(TCCR1B, CS11);  // Prescaler 256  0
    cbi(TCCR1B, CS10);  // Prescaler 256  0

    // Output Compare register 256*7 = 1792 tics => interrupt interval approx 0.0896 ms
    OCR1A = 0x0007;
}

// Bei Port C kucken wegen Buttons
//! \brief Initializes used ports of panel
void panel_init(){
	panel_startTimer();
	DDRA |= 0b00001111;
	DDRC |= 0b01000011;
	DDRD |= 0b00111111;
}

uint8_t momEbene = 0;
uint8_t momDoppelzeile = 0;

// Save DoppelSpalte im Latch
void panel_latchDisable(){
	PORTC &= 0b111111101;
}
void panel_latchEnable(){
	PORTC |= 0b00000010;
}
// Aktievieren der Ausgabe vom Latch zur Matrix (INVERTIERT)
void panel_outputDisable(){
	PORTC |= 0b01000000;
}
void panel_outputEnable(){
	PORTC &= 0b10111111;
}
// ColumnSelect oder halt fertig mit der Spalte
void panel_CLK(){
	PORTC |= 0b00000001;
	PORTC &= 0b11111110;
}
// RowSelect
void panel_setAddress(uint8_t doppelZeile){
	uint8_t test = (PORTA & 0b11100000);
	doppelZeile = doppelZeile + test;
	PORTA = doppelZeile;
}
// Farb auswahl gesamte Zeile 
void panel_setOutput(uint8_t ebene, uint8_t doppelZeile){
	for(uint8_t i=0; i<32; i++){
		PORTD = frambuffer[ebene][doppelZeile][i];
		panel_CLK();
	}
}

/*
Die Aktualisierung einer Doppelzeile läuft nun wie folgt ab: Zuerst gilt es, eine Dop-
pelzeile auszuwählen. Dann kann man gleichzeitig 6 Bits an die Eingänge der Matrix
anlegen, die Clock der Schieberegister der Chips auf 1 und wieder auf 0 ziehen, und
damit die Bits einlesen. Dieser Prozess wird 32 mal wiederholt, um alle 192 Bits in
den Schieberegistern zu füllen. Anschließend sollen die Inhalte des Schieberegisters im
Latchbaustein gespeichert werden. Dazu muss zunächst LE auf 1 und dann wieder auf
0 gezogen werden. Nun kann durch Aktivieren von OE der gespeicherte Inhalt auf die
LEDs ausgegeben werden
*/
//! \brief ISR to refresh LED panel, trigger 1 compare match interrupts
ISR(TIMER1_COMPA_vect) {
	panel_stopTimer();
	//fuck this code
	while(momDoppelzeile<16){
		panel_setAddress(momDoppelzeile);
		panel_setOutput(momEbene,momDoppelzeile);
		panel_latchEnable();
		panel_latchDisable();
		panel_outputEnable();
		panel_outputDisable();
		momDoppelzeile ++;
	}
	momDoppelzeile = 0;
	momEbene++;
	while(momDoppelzeile<16){
		panel_setAddress(momDoppelzeile);
		panel_setOutput(momEbene,momDoppelzeile);
		panel_latchEnable();
		panel_latchDisable();
		panel_outputEnable();
		panel_outputDisable();
		momDoppelzeile ++;
	}
	momEbene--;
	momDoppelzeile = 0;
	while(momDoppelzeile<16){
		panel_setAddress(momDoppelzeile);
		panel_setOutput(momEbene,momDoppelzeile);
		panel_latchEnable();
		panel_latchDisable();
		panel_outputEnable();
		panel_outputDisable();
		momDoppelzeile ++;
	}
	momDoppelzeile = 0;
	momEbene = 0;
	panel_startTimer();
}
