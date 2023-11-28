/*
 * os_memheap_drivers.h
 *
 * Created: 28.11.2023 16:12:02
 *  Author: nikum
 */ 


#ifndef OS_MEMHEAP_DRIVERS_H_
#define OS_MEMHEAP_DRIVERS_H_

//Datentypen

typedef struct{}Heap;
typedef enum{}AllocStrategy;
#define intHeap 

void os_initHeaps(void);
uint8_t os_getHeapListLenght(void);
Heap* os_lookupHeap(uint8_t index);



#endif /* OS_MEMHEAP_DRIVERS_H_ */