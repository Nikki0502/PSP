/*
 * os_memheap_drivers.c
 *
 * Created: 28.11.2023 16:11:32
 *  Author: nikum
 */ 

#include "os_memheap_drivers.h"
#include "atmega644constants.h"
#include "util.h"
#include "os_mem_drivers.h"
#include "defines.h"
#include "os_memory.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//Globale Variable
Heap intHeap__= {
		.driver = intSRAM,
		.startaddrMap = 0x100 + HEAPOFFSET ,
		.startaddrUse = (((BOTTOM_OF_PROCS_STACK - STACK_SIZE_PROC * 8)-0x100 + HEAPOFFSET)/3)+(0x100 + HEAPOFFSET),
		.endHeap= BOTTOM_OF_PROCS_STACK - STACK_SIZE_PROC * 8 ,
		.currentStrat = OS_MEM_FIRST,
		.name = "HEAP"
	};

//Funktionen
void os_initHeaps(void){
	for (MemAddr i = intHeap__.startaddrMap; i < intHeap__.startaddrUse;i++){
		intHeap__.driver->write(i,0);
	}
}
uint8_t os_getHeapListLenght(void){
	return 1;
}
Heap* os_lookupHeap(uint8_t index){
	return intHeap;
}
