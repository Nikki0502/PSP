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
	MemAddr endHeap;
	AllocStrategy currentStrat;
	char *name;
	}Heap;
	
#define intHeap (&intHeap__) 

void os_initHeaps(void);
size_t os_getHeapListLength(void);
Heap* os_lookupHeap(uint8_t index);



#endif /* OS_MEMHEAP_DRIVERS_H_ */