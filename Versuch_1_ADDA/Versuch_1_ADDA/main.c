#include <avr/io.h>
#include <stdio.h>
#include "os_input.h"
#include "adda.h"

int main(void) {
    os_initInput();
	initLEDandR2R();
	uint8_t var = 1;// "Es genügt, das zu startende Unterprogramm im Sourcecode festlegen zu koennen"
	switch (var){
		case 1:
			manuell();
		case 2:
			tracking();
		case 3:	
			sar();
	}
	return 0;
}


