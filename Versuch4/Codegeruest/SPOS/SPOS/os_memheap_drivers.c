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
#include <avr/pgmspace.h>

const PROGMEM char intName[] = "intHeap";
const PROGMEM char extName[] = "extHeap";

//Heap struct mit charakteristischen Attributen
Heap intHeap__= {
		.driver = intSRAM,
		.sizeHeap = (size_t)(AVR_MEMORY_SRAM/2 - HEAPOFFSET),
		.sizeMap =(size_t)((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3),
		.sizeUser = (size_t)(((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)*2),
		.startaddrMap = (MemAddr)(0x100 + HEAPOFFSET),
		.startaddrUse =(MemAddr)(0x100 + HEAPOFFSET + ((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)),
		.currentStrat = OS_MEM_FIRST,
		.name = intName,
		.lastAllocLeader = (MemAddr)(0x100 + HEAPOFFSET + ((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)),
		.allocFrameStart = (MemAddr)(0x100 + HEAPOFFSET + ((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)),
		.allocFrameEnd = (MemAddr)(0x100 + HEAPOFFSET + ((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)) + (((AVR_MEMORY_SRAM/2 - HEAPOFFSET)/3)*2)
};

//Heap struct mit charakteristischen Attributen
//0xFFF = Speichergröße
Heap extHeap__ = {
	.driver = extSRAM,
	.sizeHeap = (size_t)(0xFFFF),
	.sizeMap = (size_t)((0xFFFF)/3),
	.sizeUser = (size_t)(((0xFFFF)/3)*2),
	.startaddrMap= (MemAddr)(0x00),// nicht sicher
	.startaddrUse= (MemAddr)(((0xFFFF)/3)),
	.currentStrat = OS_MEM_FIRST,
	.name = extName,
	.lastAllocLeader = (MemAddr)(((0xFFFF)/3)),
	.allocFrameStart = (MemAddr)(((0xFFFF)/3)),
	.allocFrameEnd = (MemAddr)(((0xFFFF)/3))+(((0xFFFF)/3)*2)
};

//Initialisiert die Map(setzt alle Einträge auf 0(unused))
void os_initHeap(Heap* heap){
	heap->driver->init();
	for (size_t i= 0; i<heap->sizeMap ;i++){
		heap->driver->write(i + heap->startaddrMap,0x00);
	}
}

void os_initHeaps(void){
	os_initHeap(intHeap);
	os_initHeap(extHeap);
}

size_t os_getHeapListLength(void){
	// ???
	return 2;
}
Heap* os_lookupHeap(uint8_t index){
	if (index == 1){
		return extHeap;
	}
	return intHeap;
}
