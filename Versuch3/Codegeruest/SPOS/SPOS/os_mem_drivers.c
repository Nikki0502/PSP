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

//Globale Var
MemDriver intSRAM__ = {
	.addr_start = AVR_SRAM_START;
	.addr_end = AVR_SRAM_END;
	.init()=&init();
	.read()=&read();
	.write()=&write();
	.name = "SRAMi";
	.addr_curr = AVR_SRAM_LAST;
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


