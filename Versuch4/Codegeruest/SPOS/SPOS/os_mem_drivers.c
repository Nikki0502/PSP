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

#include <avr/interrupt.h>
#include <stdbool.h>

//Funktionen des MemDriver-Types(init,read,write)

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

void Init_External(void){
	os_spi_int();
     //extSRAM soll kein slave sein
	 
	 //Byte Operationen aktivieren mit Write Mode Register
	os_spi_send(0b00000001); //Write MODE Register
	os_spi_send(0b00000000);
	 
	
}

MemValue read_External(MemAddr addr){
	
}

void write_External(MemAddr addr, MemValue value){
	
}
//Driver intSRAM init 
MemDriver intSRAM__ = {
	.startAddr = 0x100 + HEAPOFFSET,
	.endAddr = 0x10FF,
	.init = init_Internal,
	.read = read_Internal,
	.write = write_Internal
	//.name = "SRAM"
};
//Driver extSRAM
MemDriver extSRAM__ = {
	.startAddr = 0x0,
	.endAddr = 0xF9F, //64000 bit / 16 bit Adressen == 4000 Adressen also letzte Adresse = 3999
	.init = Init_External,
	.read = read_External,
	.write = write_External,
};
	


