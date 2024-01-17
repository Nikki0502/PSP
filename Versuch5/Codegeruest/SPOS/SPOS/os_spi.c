/*
 * os_spi.c
 *
 * Created: 13.12.2023 11:12:32
 *  Author: trisi
 */ 
#include "os_spi.h"
#include "atmega644constants.h"
#include "os_scheduler.h"
void os_spi_int(void){
	// B7 = CLK, B6 = MISO, B5 = MOSI, B4 = \CS, B3 = SIO2(Nicht für den Versuch relevant)
	DDRB |=  0b00010000;
	DDRB |=  0b10110000;
	DDRB &= 0b10111111;
	//PORTB = 0b01000000;
	// SPI Register 
	// SPI Control Register
	// SPIE=0(keine IR) SPE=1(Activate Spi module) DORD=0(MSB zu erst) MSTR=1(AVR als Master) CPOL=0(Idle Low, Active High) CPHA=0(Leading Edge) SPR1=0(Takt) SPR0=0(Takt)
	SPCR = 0b01010000;
	// SPI Status Register
	// SPIF=0(abschull der uebertragung signal, noch nichts uebertragen also 0) WCOL=0(nicht bedeutend) - - - - - SPI2X=1(takt)
	SPSR = 0b00000001;
	
	// zum takt: fcpu = 20mhz , max 23LC1024takt = 20mhz
	// SPI2X SPR1 SPR0 SPI-Frequenz
	// 1	 0	  0	   fCPU/2
	// so max einstellbare spi freqeunz
	
}
/* 
 * Parameters
 * data The byte which is send to the slave
 * Returns
 * The byte received from the slave
 */
uint8_t os_spi_send(uint8_t data){
	// darf nicht unterbrochen werden
	os_enterCriticalSection();
	// SPI Data Register (SPDR) setzen
	SPDR = data; 
	// warten auf ende der uebertragung singaliesert von SPIF 
	while((SPSR & 0b10000000) == 0x00){} 
	uint8_t byte_from_slave = SPDR;
	os_leaveCriticalSection();
	return byte_from_slave;
}
/*
 * Returns
 * The byte received from the slave
 * calls os_spi_send
 */
uint8_t os_spi_receive() {
	//darf nicht unterbrochen werden
	os_enterCriticalSection();
	// idk warum das so funkt aber laut aufgaben stellung und doxxygen sollte das richtig sein 
	uint8_t byte_from_slave = os_spi_send(0xFF);
	os_leaveCriticalSection();
	return byte_from_slave;
}