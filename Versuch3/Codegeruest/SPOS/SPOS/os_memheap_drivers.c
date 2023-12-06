/*
 * os_memheap_drivers.c
 *
 * Created: 28.11.2023 16:11:32
 *  Author: nikum
 */ 

#include "os_memheap_drivers.h"
#include "defines.h"


#include <avr/interrupt.h>
#include <stdbool.h>

//Globale Variable
Heap intHeap__= {
		.driver = intSRAM,
		.sizeHeap = (size_t)(AVR_MEMORY_SRAM/2 - HEAPOFFSET),
		.sizeMap =(size_t)((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3),
		.sizeUser = (size_t)(((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)*2),
		.startaddrMap = (MemAddr)(0x100 + HEAPOFFSET),
		.startaddrUse =(MemAddr)(0x100 + HEAPOFFSET + ((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)),
		.currentStrat = OS_MEM_FIRST,
		.name = "HEAP"
	};

//Funktionen
void os_initHeaps(void){
	for (size_t i= 0; i<intHeap__.sizeMap ;i++){
		intHeap__.driver->write(i + intHeap__.startaddrMap,0x00);
	}
}
size_t os_getHeapListLength(void){
	return 1;
}
Heap* os_lookupHeap(uint8_t index){
	return intHeap;
}
