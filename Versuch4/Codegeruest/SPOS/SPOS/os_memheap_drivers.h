/*
 * os_memheap_drivers.h
 *
 * Created: 28.11.2023 16:12:02
 *  Author: nikum
 */ 


#ifndef OS_MEMHEAP_DRIVERS_H_
#define OS_MEMHEAP_DRIVERS_H_
#include "os_mem_drivers.h"
#include <stddef.h>

//Datentypen
typedef enum{
	OS_MEM_FIRST,
	OS_MEM_NEXT,
	OS_MEM_BEST,
	OS_MEM_WORST
}AllocStrategy;

typedef struct{
	MemDriver *driver;
	MemAddr startaddrMap;
	MemAddr startaddrUse;
	MemAddr endaddrHeap;
	size_t sizeMap;
	size_t sizeUser;
	size_t sizeHeap;
	AllocStrategy currentStrat;
	char *name;
	MemAddr lastAllocLeader;
}Heap;

Heap intHeap__;
Heap extHeap__;
#define intHeap (&intHeap__) 
#define extHeap (&extHeap__)

void os_initHeap(Heap* heap);
void os_initHeaps(void);
size_t os_getHeapListLength(void);
Heap* os_lookupHeap(uint8_t index);



#endif /* OS_MEMHEAP_DRIVERS_H_ */