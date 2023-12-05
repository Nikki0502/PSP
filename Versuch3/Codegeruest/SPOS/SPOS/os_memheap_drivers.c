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
		.startaddrMap = (MemAddr) (0x100 + HEAPOFFSET) ,
		.startaddrUse = (MemAddr)((((AVR_MEMORY_SRAM/2) - (0x100 + HEAPOFFSET))/3)+(0x100 + HEAPOFFSET)),
		.endaddrHeap= (MemAddr)(AVR_MEMORY_SRAM/2) ,
		.sizeMap= (((AVR_MEMORY_SRAM/2) - (0x100 + HEAPOFFSET))/3)+(0x100 + HEAPOFFSET) -  (0x100 + HEAPOFFSET),
		.sizeUser= (AVR_MEMORY_SRAM/2) - (((AVR_MEMORY_SRAM/2) - (0x100 + HEAPOFFSET))/3)+(0x100 + HEAPOFFSET),
		.currentStrat = OS_MEM_FIRST,
		.name = "HEAP"
	};

//Funktionen
void os_initHeaps(void){
	for (MemAddr i = intHeap__.startaddrMap; i < intHeap__.startaddrUse;i++){
		intHeap__.driver->write(i,0);
	}
}
size_t os_getHeapListLength(void){
	return 1;
}
Heap* os_lookupHeap(uint8_t index){
	return intHeap;
}
