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

static void init(void){
}

static MemValue read(MemAddr addr){
	uint8_t *pointer = (uint8_t*)&addr;
	MemValue value = *pointer;
	return value;
}

static void write(MemAddr addr , MemValue value){
	uint8_t *pointer = (uint8_t*)&addr;
	*pointer = value;
}

//Driver intSRAM init 
MemDriver intSRAM__ = {
	.startAddr = AVR_SRAM_START,
	.endAddr = AVR_SRAM_END,
	.currAddr = AVR_SRAM_LAST,
	.remainingBytesInSRAM = AVR_MEMORY_SRAM,
	.name = "SRAM",
	.init = &init,
	.read = &read,
	.write = &write
};
	


