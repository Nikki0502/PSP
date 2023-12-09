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

//Driver intSRAM init 
MemDriver intSRAM__ = {
	.startAddr = 0x100 + HEAPOFFSET,
	.endAddr = 0x10FF,
	.init = init_Internal,
	.read = read_Internal,
	.write = write_Internal
	//.name = "SRAM"
};
	


