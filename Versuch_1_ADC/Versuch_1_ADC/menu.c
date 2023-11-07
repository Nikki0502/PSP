#include "menu.h"

#include "adc.h"
#include "bin_clock.h"
#include "lcd.h"
#include "led.h"
#include "os_input.h"

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

/*!
 *  Hello world program.
 *  Shows the string 'Hello World!' on the display.
 */
void helloWorld(void) {
  while (( os_getInput() & 0b00001000 ) != 0b00001000 ){// check if ESC is pressed
	  //lcd_writeString("Hallo Welt!");
	  lcd_writeProgString(PSTR("Hallo Welt!"));
	  _delay_ms(500);
	  lcd_clear();
	  _delay_ms(500);
  }
  while(( os_getInput() & 0b00001000 ) == 0b00001000){};// checks if ESC is no longer pressed 
  showMenu();// return to main menu
}

/*!
 *  Shows the clock on the display and a binary clock on the led bar.
 */
void displayClock(void) {
  while (( os_getInput() & 0b00001000 ) != 0b00001000 ){// check if ESC is pressed
	uint16_t clockVal=0b0000000000000000;
	clockVal |= ((uint16_t)getTimeHours() <<12) | ((uint16_t)getTimeMinutes() << 6) | (uint16_t)getTimeSeconds() ;  // set clockVal to Hour<12 Min<6 Sec
	setLedBar(clockVal);// checkt und das ist richtig lcd_writeHex(clockVal);
	uint8_t hours = getTimeHours();
	uint8_t minutes = getTimeMinutes();
	uint8_t seconds = getTimeSeconds();
	uint16_t milliseconds = getTimeMilliseconds();
	char formattedTime[12]; 
	sprintf(formattedTime, "%02d:%02d:%02d:%03d", hours, minutes, seconds, milliseconds);
	lcd_writeString(formattedTime);
	lcd_clear();
  }
  // checks if ESC is no longer pressed 
  while(( os_getInput() & 0b00001000 ) == 0b00001000){};
  showMenu();// return to main menu
}

/*!
 *  Shows the stored voltage values in the second line of the display.
 */
void displayVoltageBuffer(uint8_t displayIndex) {
	//Vorbereiten 
	lcd_line2();
	lcd_erase(2);
	//Ausgabe der Spannung an displayIndex im Buffer
	char formattedIndex[4];
	snprintf(formattedIndex,sizeof(formattedIndex), "%03d", displayIndex+1);
	lcd_writeString(formattedIndex);
	lcd_writeProgString(PSTR("/"));
	lcd_writeProgString(PSTR("100: "));
	uint16_t storedVoltage = getStoredVoltage(displayIndex);
	lcd_writeVoltage(storedVoltage,1023,5);
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {
	//Vorbereitung
	lcd_clear();
	uint8_t bufferIndex=0;  
	//Solange esc nicht gedrückt ist soll das Programm laufen
	while (( os_getInput() & 0b00001000 ) != 0b00001000 ){
		_delay_ms(100);//gegen Flackern
		//Anzeige der Spannung in der erstel Zeile des Displays
		lcd_line1();
		lcd_erase(1);
		lcd_writeProgString(PSTR("Voltage:"));
		lcd_writeVoltage(getAdcValue(),1023,5);
		//Ausgabe der Spannung mit LEDBar
		//Konvertier die AdcValue zu ausgebbaren Wert mit 0,3V pro LED
		uint16_t ledValue =0b00000000;
		uint16_t adcResult = getAdcValue();
		while(adcResult >= 68){
			ledValue = ledValue << 1;
			ledValue = ledValue + 0b00000010;
			adcResult = adcResult -68;
		}
		setLedBar(ledValue);
		//Speichern von Messwerten
		//Wenn Enter ist gedrueckt
		if((os_getInput() & 0b00000001) == 0b00000001){
			storeVoltage();
		}
		//Wenn UP ist gedrueckt
		if((os_getInput() & 0b00000100)== 0b00000100){
			if(bufferIndex<getBufferIndex()-1){
				bufferIndex +=1;
				//displayVoltageBuffer(bufferIndex);
			}
		}
		//Wenn DOWN ist gedrueckt
		if((os_getInput() & 0b00000010)== 0b00000010){
			if(bufferIndex>0){
				bufferIndex -=1;
				//displayVoltageBuffer(bufferIndex);
			}
		}
		displayVoltageBuffer(bufferIndex);		
	}
	// checks if ESC is no longer pressed
	while(( os_getInput() & 0b00001000 ) == 0b00001000){};
	showMenu();// return to main menu
  
}

/*! \brief Starts the passed program
 *
 * \param programIndex Index of the program to start.
 */
void start(uint8_t programIndex) {
    // Initialize and start the passed 'program'
    switch (programIndex) {
        case 0:
            lcd_clear();
            helloWorld();
            break;
        case 1:
            activateLedMask = 0xFFFF; // Use all LEDs
            initLedBar();
            initClock();
            displayClock();
            break;
        case 2:
            activateLedMask = 0xFFFE; // Don't use LED 0
            initLedBar();
            initAdc();
            displayAdc();
            break;
        default:
            break;
    }

    // Do not resume to the menu until all buttons are released
    os_waitForNoInput();
}

/*!
 *  Shows a user menu on the display which allows to start subprograms.
 */
void showMenu(void) {
    uint8_t pageIndex = 0;

    while (1) {
        lcd_clear();
        lcd_writeProgString(PSTR("Select:"));
        lcd_line2();

        switch (pageIndex) {
            case 0:
                lcd_writeProgString(PSTR("1: Hello world"));
                break;
            case 1:
                lcd_writeProgString(PSTR("2: Binary clock"));
                break;
            case 2:
                lcd_writeProgString(PSTR("3: Internal ADC"));
                break;
            default:
                lcd_writeProgString(PSTR("----------------"));
                break;
        }

        os_waitForInput();
        if (os_getInput() == 0b00000001) { // Enter
            os_waitForNoInput();
            start(pageIndex);
        } else if (os_getInput() == 0b00000100) { // Up
            os_waitForNoInput();
            pageIndex = (pageIndex + 1) % 3;
        } else if (os_getInput() == 0b00000010) { // Down
            os_waitForNoInput();
            if (pageIndex == 0) {
                pageIndex = 2;
            } else {
                pageIndex--;
            }
        }
    }
}
