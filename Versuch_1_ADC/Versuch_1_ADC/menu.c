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
// Repeat until ESC gets pressed
  while (( os_getInput() & 0b00001000 ) != 0b00001000 ){
	  lcd_writeString("Hallo Welt!");
	  //lcd_writeProgString(PSTR"Hallo Welt!")
	  _delay_ms(500);
	  lcd_clear();
	  _delay_ms(500);
  }
  while(( os_getInput() & 0b00001000 ) == 0b00001000){};//Sobald der ESC-Button gedrückt und losgelassen wurde
  showMenu();//soll ins Hauptmenü zurückgekehrt werden
}

/*!
 *  Shows the clock on the display and a binary clock on the led bar.
 */
void displayClock(void) {
  while (( os_getInput() & 0b00001000 ) != 0b00001000 ){
	uint16_t clockVal=0b0000000000000000;
	clockVal |= getTimeHours() <<12  ;  // kcuk mal hier
	clockVal |= getTimeMinutes() << 6:
	clockVal |=getTimeSeconds() ;
	setLedBar(clockVal);
	lcd_writeProgString(PSTR);// Zahlen in das Format HH:MM:SS:mmm transformiert werden
  }
  while(( os_getInput() & 0b00001000 ) == 0b00001000){};
  showMenu();
}

/*!
 *  Shows the stored voltage values in the second line of the display.
 */
void displayVoltageBuffer(uint8_t displayIndex) {
#warning IMPLEMENT STH. HERE
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {
#warning IMPLEMENT STH. HERE
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
