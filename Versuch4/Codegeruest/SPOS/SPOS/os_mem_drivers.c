/*
 * os_mem_drivers.c
 *
 * Created: 28.11.2023 16:07:00
 *  Author: nikum
 */ 
#include "os_mem_drivers.h"
#include "atmega644constants.h"
#include "util.h"
#include "defines.h"
#include "os_spi.h"
#include "os_scheduler.h"

#include <avr/interrupt.h>
#include <stdbool.h>

#define WRMR 0x01 // Write Mode Register
#define READ 0x03 // Read data from memory array beginning at selected address
#define WRITE 0x02 // Write data to memory array beginning at selected address
#define ByteMode 0x00 // Bitweiser modus

// Hilfsfunktionenn
// Activates the external SRAM as SPI slave. 
void select_memory (){
	// by bringing CS low 
	//DDRB &=  0b11101111;
	DDRB |=  0b00010000;
}
// Deactivates the external SRAM as SPI slave. 
void deselect_memory (){
	// by bringing CS high
	//DDRB |=  0b00010000;
	DDRB &=  0b11101111;
}
// Sets the operation mode of the external SRAM.
void set_operation_mode (uint8_t mode){
	os_enterCriticalSection();
	select_memory();
	os_spi_send(WRMR);
	os_spi_send(mode);
	deselect_memory();
	os_leaveCriticalSection();
}
// Transmitts a 24bit memory address to the external SRAM.
void transfer_address (MemAddr addr){
	// sendet die 7 idc bits und noch ein leeres bit, da wir nicht den ganzen speicher nutzen
	os_spi_send(0x0);
	// sendet eigentliche addr
	os_spi_send(addr>>8);
	os_spi_send((uint8_t)addr);
}

// Funktionen des MemDriver-Types(init,read,write)
// Interner SRAM
void init_Internal(void){
}
//Gibt den Wert an der übergebenen Adresse zurück
MemValue read_Internal(MemAddr addr){
	uint8_t *pointer = (uint8_t*)addr;
	MemValue value = *pointer;
	return value;
}
//Beschreibt den Wert der übergebenen Adresse
void write_Internal(MemAddr addr , MemValue value){
	uint8_t *pointer = (uint8_t*)addr;
	*pointer = value;
}

// Externer SRAM
void init_External(void){	
	// SPI-Modul aktiviert werden
	os_spi_int();
	// SRAM nicht als SPI-Slave selektiert ist
	deselect_memory();
	// byteweisen Zugriffsmodus
	set_operation_mode(ByteMode);
}

MemValue read_External(MemAddr addr){
	os_enterCriticalSection();
	select_memory();
	os_spi_send(READ);
	transfer_address(addr);
	uint8_t data = os_spi_receive();
	deselect_memory();
	os_leaveCriticalSection();
	return data;
}

void write_External(MemAddr addr, MemValue value){
	os_enterCriticalSection();
	select_memory();
	os_spi_send(WRITE);
	transfer_address(addr);
	os_spi_send(value);
	deselect_memory();
	os_leaveCriticalSection();
}

//Driver intSRAM  
MemDriver intSRAM__ = {
	.startAddr = 0x100 + HEAPOFFSET,
	.endAddr = 0x10FF,
	.init = init_Internal,
	.read = read_Internal,
	.write = write_Internal
};
//Driver extSRAM
MemDriver extSRAM__ = {
	.startAddr = 0x0,
	.endAddr = 0xFFFF, // da 64 KiB = 64*1024 Byte und noch -1 wegen start 0
	.init = init_External,
	.read = read_External,
	.write = write_External
};
	


