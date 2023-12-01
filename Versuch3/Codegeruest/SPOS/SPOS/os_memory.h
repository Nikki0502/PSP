/*
 * os_memory.h
 *
 * Created: 28.11.2023 16:13:19
 *  Author: nikum
 */ 


#ifndef OS_MEMORY_H_
#define OS_MEMORY_H_

#include "defines.h"
#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"

MemAddr os_malloc(Heap* heap, uint16_t size);

void os_free(Heap* heap, MemAddr addr);

size_t os_getUseSize(Heap const* heap);
	
size_t os_getMapSize(Heap const* heap);
	
MemAddr os_getMapStart(Heap const* heap);
	
MemAddr os_getUseStart(Heap const* heap);
	
uint16_t os_getChunkSize(Heap const* heap, MemAddr addr);

void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat);

AllocStrategy os_getAllocationStrategy(Heap const* heap);

void setLowNibble (const Heap *heap, MemAddr addr, MemValue value);

void setHighNibble (const Heap *heap, MemAddr addr, MemValue value);

MemValue getLowNibble (const Heap *heap, MemAddr addr);

MemValue getHighNibble (const Heap *heap, MemAddr addr);

#endif /* OS_MEMORY_H_ */