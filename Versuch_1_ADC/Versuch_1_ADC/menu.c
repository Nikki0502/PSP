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
	// set courser to line 2 and erase this line before writing again  
	lcd_line2();
	lcd_erase(2);
	//get the storedVoltage that should be displaed
	uint16_t storedVoltage = getStoredVoltage(displayIndex);
	//display the Voltage in a 54/100 Voltag Format
	char voltString[3];
	snprintf(voltString,sizeof(voltString), "%03d",getBufferIndex()+1);
	lcd_writeString(voltString);
	lcd_writeProgString(PSTR("/"));
	lcd_writeChar(getBufferSize());
	lcd_writeVoltage(storedVoltage,1023,5);
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {
  // bufferindex for what should be displayed next 
  uint8_t bufferindex = 0;
  while (( os_getInput() & 0b00001000 ) != 0b00001000 ){
	  // prepare to write in line 1 
	  lcd_line1();
	  lcd_erase(1);
	  // write Voltage on LCD
	  lcd_writeProgString(PSTR("Voltage:"));
	  lcd_writeVoltage(getAdcValue(),1023,5);
	  //convert Voltage to displayble number for LedBar 
	  uint16_t ledValue =0b00000000;
	  uint16_t adcResult = getAdcValue();
	  while(adcResult >= 68){
		  ledValue = ledValue << 1;
		  ledValue = ledValue + 0b00000010;
		  adcResult = adcResult -68;
	  }
	  // bitmask already set for 15 leds
	  setLedBar(ledValue);
	  // options for user input
	  // press enter to save voltage 
	  if((os_getInput() & 0b00000001) == 0b00000001){
		  storeVoltage();
	  }
	  // press up to increment bufferindex and display next voltage in buffer 
	  if((os_getInput() & 0b00000100)== 0b00000100 && getBufferIndex()> bufferindex){
		  //up
		  bufferindex = bufferindex + 1;
		  displayVoltageBuffer(bufferindex);
	  }
	  // press down to decrement bufferindex and display last voltage in buffer 
	  if((os_getInput() & 0b00000010)== 0b00000010 && bufferindex >0 ){
		  //down
		 bufferindex = bufferindex - 1;
		 displayVoltageBuffer(bufferindex);
	  }
	  _delay_ms(100);
  }
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
