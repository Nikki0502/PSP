/*
 * os_mem_drivers.c
 *
 * Created: 28.11.2023 16:07:00
 *  Author: nikum
 */ 
#include "os_mem_drivers.h"
#include "util.h"
#include "defines.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//Driver intSRAM init 
MemDriver intSRAM__ = {
	.startAddr = AVR_SRAM_START,
	.endAddr = AVR_SRAM_END,
	.currAddr = AVR_SRAM_LAST,
	.remainingBytesInSRAM =AVR_MEMORY_SRAM,
	.name = "SRAM",
	.init = &init,
	.read = &read,
	.write = &write
};
	
//Funktionen des MemDriver-Types(init,read,write)

void init(void){	
}

MemValue read(MemAddr addr){
	MemValue value = *addr;
	return value;
}

void write(MemAddr addr , MemValue value){
	*addr = value;
}


