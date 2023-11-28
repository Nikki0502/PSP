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

size_t os_get{Map,Use}Size(Heap const* heap);
	
MemAddr os_get{Map,Use}Size(Heap const* heap);
	
uint16_t os_getChunkSize(Heap const* heap, MemAddr addr);

void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat);

AllocStrategy os_getAllocationStrategy(Heap* const* heap);

#endif /* OS_MEMORY_H_ */