/*
 * os_memory.c
 *
 * Created: 28.11.2023 16:12:59
 *  Author: nikum
 */ 
#include "os_memory.h"
#include "atmega644constants.h"
#include "util.h"
#include "defines.h"
#include "os_scheduler.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//Funktionen
MemAddr os_malloc(Heap* heap, uint16_t size){
	os_enterCriticalSection();
	
	os_leaveCriticalSection();
}
	
void os_free(Heap* heap, MemAddr addr){
	os_enterCriticalSection();
	os_leaveCriticalSection();
}
	
size_t os_getUseSize(Heap const* heap){
	return heap->endHeap - heap->startaddrUse;
}

size_t os_getMapSize(Heap const* heap){
	return heap->startaddrUse - heap->startaddrMap;
}

MemAddr os_getMapStart(Heap const* heap){
	return heap->startaddrMap;
}

MemAddr os_getUseStart(Heap const* heap){
	return heap->startaddrUse;
}

	
uint16_t os_getChunkSize(Heap const* heap, MemAddr addr){
}
	
void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat){
	heap->currentStrat = allocStrat;
}

	
AllocStrategy os_getAllocationStrategy(Heap* const* heap){
	return heap->currentStrat;
}
	
void setLowNibble (const Heap *heap, MemAddr addr, MemValue value){
	if(addr < heap->startaddrUse && addr >= heap->startaddrMap){
		//*addr = (0b11110000 & *addr);
		//*addr |= (0b00001111 & value);
		heap->driver->write(addr,(getHighNibble(addr)<<4)+value));
	}
}

void setHighNibble (const Heap *heap, MemAddr addr, MemValue value){
	if(addr < heap->startaddrUse && addr >= heap->startaddrMap){
		//*addr = (0b000011111 & *addr);
		//*addr |= (value<<4);
		heap->driver->write(addr,(getLowNibble(addr)+(value<<4)));
	}
}

MemValue getLowNibble (const Heap *heap, MemAddr addr){
	return (heap->driver->read(addr)&0b00001111);
}

MemValue getHighNibble (const Heap *heap, MemAddr addr){
	return (heap->driver->read(addr)>>4);
}

	
