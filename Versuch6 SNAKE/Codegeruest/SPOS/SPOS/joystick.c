/*Ein Joystick ist ein in zwei Dimensionen bewegbarer Steuerhebel zur Richtungseingabe.
Er wird durch zwei Potentiometer realisiert, die als Spannungsteiler zwischen VCC (5 V)
und Ground (0 V) fungieren. Jedes der Potentiometer liefert somit ein analoges Signal,
das die Neigung des Hebels in eine der Achsen (horizontal oder vertikal) wiederspiegelt.
In Ruheposition befinden sich beide Potentiometer in mittiger Position, wodurch eine
Spannung von ca. 2,5 V am Ausgang anliegt. Beim Auslenken nach oben bzw. unten
liefert das Potentiometer, das zur vertikalen Achse gehört, dann eine entsprechend höhere
bzw. niedrigere Spannung bis hin zu annähernd 0 V bzw. 5 V.
Als weitere Eingabemöglichkeit kann auf den Joystick gedrückt werden, wodurch ein
Schalter geschlossen wird, der Pin A7 mit Ground verbindet (siehe Pinbelegung 6.7).*/

#include <avr/interrupt.h>
#include <stdbool.h>
#include "joystick.h"
#include "util.h"


/************************************************************************/
/*                           JOYSTICK                                   */
/************************************************************************/

/*
void js_init() Diese Funktion konfiguriert die Input-Pins sowie den internen AD-Wandler
mithilfe der Register ADMUX und ADCSRA. Es soll eine interne Referenzspannung von Vcc
benutzt werden.
*/
void js_init(){
	// Joy. Vertikal, horizontal and button as input
	DDRA &= 0b00011111;
	// pullup.res.
	// idk glaub die brauchen kein
	PORTA |= 0b10000000;
	// ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0 ADCSRA
	/*
     * ADEN    = 1      Enable ADC
     * ADSC    = 0      Used to start a conversion
     * ADATE   = 0      No continuous conversion
     * ADIF    = 0      Indicates that the conversion has finished
     * ADIE    = 0      Do not use interrupts
     * ADPS2:0 = 111    Prescaler 128 -> 20MHz / 128 = 156kHz ADC frequency
     */
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// Config ADMUX
	// REFS1 REFS0 ADLAR MUX4 MUX3 MUX2 MUX1 MUX0
	// Bit 7,6 sel. Vcc
	// Bit 5 legt Ergebnis 2xMB in ADCH und 8xLB in ADCL
	// das eine muss vor dem anderen augelesen werden glaub ADACH dann ADCL sonst tauschen
	// letzten 4 bit auswahl der zu konvertierenden Spannung
	// Input sind an Stelle 5(horizontal) und 6(vertikal) in DDRA
	ADMUX = 0b01000000;
}

/*
uint16_t js_getHorizontal() und js_getVertical() Diese Funktionen sollen mithilfe
des internen AD-Wandlers den Analogpegel je eines Potentiometers auslesen und den
gemessenen Wert zurückgeben.
*/
uint16_t js_getHorizontal(){
	// Input Auswahl
	ADMUX &= 0b11100000;
	ADMUX |= 0b00000101;
	// Konversion start
	ADCSRA |= 0b01000000;
	// warten bis ender der Conversion
	while((ADCSRA&0b01000000)==0b01000000){}
	// Output auslesen und Rueckgabe
	uint16_t val =  ADCL | ((uint16_t)ADCH << 8);
	return val;
}

uint16_t js_getVertical(){
	// Input Auswahl
	ADMUX &= 0b11100000;
	ADMUX |= 0b00000110;
	// Konversion start
	ADCSRA |= 0b01000000;
	// warten bis ender der Conversion
	while((ADCSRA&0b01000000)==0b01000000){}
	// Output auslesen und Rueckgabe
	uint16_t val =  ADCL | ((uint16_t)ADCH << 8);
	return val;
}

// kleine hilfs funktion fuer rechnen mit den Werten vom ADC
// valueUpperBound = 1023 for 10-bit value
// voltUpperBound = 5 for 5V;
float calcVoltage(uint16_t voltage){
	uint16_t valueUpperBound = 1023;
	uint8_t voltUpperBound =5;
	uint8_t intVal;
	float floatVal;
	// Calculate integer and float part of the voltage
	// idk ob das worked aber habs aus Versuch 1 kopiert
	voltage *= voltUpperBound;
	intVal = voltage / valueUpperBound;
	floatVal = (uint32_t)(voltage - (intVal * valueUpperBound))/ valueUpperBound;
	return (float)intVal + floatVal;
}

/*
Direction js_getDirection() Diese Funktion soll mithilfe von js_getHorizontal und
js_getVertical die Richtung des Joysticks ableiten können. Für die Neutralposition
soll ein Toleranzbereich von etwa 1,0 V um die Ruheposition verwendet werden.
*/
// Ruheposition ist bei ca. 2,5 V
Direction js_getDirection(){
	float horizontal = calcVoltage(js_getHorizontal());
	float vertical = calcVoltage(js_getVertical());
	if(horizontal<=3.5&&horizontal>=1.5&&vertical<=3.5&&vertical>=1.5){
		return JS_NEUTRAL;
	}
	float vertDelta = (2.5-vertical)*(2.5-vertical);
	float horiDelta = (2.5-horizontal)*(2.5-horizontal);
	if(vertDelta>horiDelta){
		if(vertical>2.5){
			return JS_UP;
		}
		else{
			return JS_DOWN;
		}
	}
	else{
		if(horizontal>2.5){
			return JS_RIGHT;
		}
		else{
			return JS_LEFT;
		}
	}
	// idk was man machen soll falls die gleich sind
	return JS_NEUTRAL;

}

/*
bool js_getButton() Diese Funktion gibt true zurück, wenn gerade auf den Joystick
gedrückt wird, d.h. der in ihm verbaute Schalter geschlossen ist. Andernfalls gibt diese
Funktion false zurück.
*/
bool js_getButton(){
	// Buttons sollten reversed sein. zu mindestens waren das die alten
	return ((~PINA & 0b10000000)>>7)==1;
}



