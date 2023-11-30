/*
 * os_memheap_drivers.h
 *
 * Created: 28.11.2023 16:12:02
 *  Author: nikum
 */ 


#ifndef OS_MEMHEAP_DRIVERS_H_
#define OS_MEMHEAP_DRIVERS_H_
#include "os_mem_drivers.h"

//Datentypen

typedef struct{
	MemDriver *driver;
	uint16_t startaddrMap;
	uint16_t startaddrUse;
	uint16_t endHeap;
	AllocStrategy currentStrat;
	char *name[];
	}Heap;
	
typedef enum{
	OS_MEM_FIRST,
	OS_MEM_NEXT,
	OS_MEM_BEST,
	OS_MEM_WORST
	}AllocStrategy;
	
#define intHeap (&intHeap__) 

void os_initHeaps(void);
uint8_t os_getHeapListLenght(void);
Heap* os_lookupHeap(uint8_t index);



#endif /* OS_MEMHEAP_DRIVERS_H_ */