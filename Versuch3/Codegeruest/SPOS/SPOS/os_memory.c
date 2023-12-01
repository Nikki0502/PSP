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
#include "os_memheap_drivers.h"


#include <avr/interrupt.h>
#include <stdbool.h>

//Funktionen
MemAddr os_malloc(Heap* heap, uint16_t size){
	os_enterCriticalSection();
	MemAddr freeAddrUser;
	switch (os_getAllocationStrategy(heap)){
		case OS_MEM_FIRST: freeAddrUser = os_Memory_FirstFit(heap,(size_t)size); break;
		default: freeAddrUser= 0;
	}
	if(freeAddrUser==0){
		return 0;
	}
	MemAddr freeAddrMap = os_getMapStart() + ((freeAddrUser - os_getUseStart())/2);
	bool highNible =(freeAddrUser - os_getUseStart())%2==0;
	// markiern von Map
	for(uint16_t i = 0; i< size; i++){
		if(i==0){
			if(highNible){
				setHighNibble(heap,freeAddrMap,os_getCurrentProc());
				highNible = false;
			}
			else{
				setLowNibble(heap,freeAddrMap,os_getCurrentProc());
				highNible = true;
			}
		}
		else{
			if(highNible){
				setHighNibble(heap,freeAddrMap,(MemValue)0xF);
				highNible = false;
			}
			else{
				setLowNibble(heap,freeAddrMap,(MemValue)0xF);
				highNible = true;
			}
		}
		if(highNible == false){
			freeAddrMap +=1;
		}
	}
	return freeAddrUser;
	os_leaveCriticalSection();
}
	
void os_free(Heap* heap, MemAddr addr){
	os_enterCriticalSection();
	MemAddr usedAddrMap = (addr -os_getUseStart())/2;
	bool highNible =(addr-os_getUseStart())%2==0;
	while()
	os_leaveCriticalSection();
}
	
size_t os_getUseSize(Heap const* heap){
	return (size_t)(heap->endHeap - heap->startaddrUse);
}

size_t os_getMapSize(Heap const* heap){
	return (size_t)(heap->startaddrUse - heap->startaddrMap);
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

MemAddr os_getMapAddr(const Heap *heap, MemAddr addr){
	return os_getMapStart() + ((addr - os_getUseStart())/2);
}

void os_setMapAddr(const Heap *heap, MemAddr addr, MemValue value){
	MemAddr freeAddrMap = os_getMapStart() + ((addr - os_getUseStart())/2);
	bool highNible =(addr - os_getUseStart())%2==0;
	if(highNible){
		setHighNibble(heap,freeAddrMap,value);
	}
	else{
		setLowNibble(heap,freeAddrMap,value);
	}
}

	
