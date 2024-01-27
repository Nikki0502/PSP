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


//Wie kann man input an zum Beispiel Joy horizontal (A5) lesen?

void js_init(){
	DDRA &= 0b00011111;
	PORTA |= 0b11100000;
	ADMUX |= 0b01000000;
	ADMUX &= 0b01011111;
	ADCSRA |= 0b10000000;
}

uint16_t js_getHorizontal(){
	ADCSRA |= 0b01000000;
	
	ADCSRA &= 0b10111111;
	
}

uint16_t js_getVertical(){
	ADCSRA |= 0b01000000;
	
	ADCSRA &= 0b10111111;
}

Direction js_getDirection(){
	ADCSRA |= 0b01000000;
	
	ADCSRA &= 0b10111111;
}

bool os_getButton(){
	ADCSRA |= 0b01000000;
	
	ADCSRA &= 0b10111111;
}