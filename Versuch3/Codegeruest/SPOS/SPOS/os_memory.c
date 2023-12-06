/*
 * os_memory.c
 *
 * Created: 28.11.2023 16:12:59
 *  Author: nikum
 */ 
#include "os_memory.h"
#include "os_scheduler.h"
#include "os_core.h"
#include "os_memory_strategies.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>


/*
 * Nible Management
 */
void setLowNibble (const Heap *heap, MemAddr addr, MemValue value){
	if(addr < heap->startaddrUse && addr >= heap->startaddrMap){
		heap->driver->write(addr, ((getHighNibble(heap,addr)<<4) | value));
	}
}

void setHighNibble (const Heap *heap, MemAddr addr, MemValue value){
	if(addr < heap->startaddrUse && addr >= heap->startaddrMap){
		heap->driver->write(addr, ((getLowNibble(heap,addr) | (value<<4))));
	}
}

MemValue getLowNibble (const Heap *heap, MemAddr addr){
	MemValue volatile value =(heap->driver->read(addr) & 0b00001111);
	return value;
}

MemValue getHighNibble (const Heap *heap, MemAddr addr){
	MemValue value = (heap->driver->read(addr)>>4);
	return value;
}




/*
 *	Map Management
 */

//Start of the Map
MemAddr os_getMapStart(const Heap* heap){
	return heap->startaddrMap;
}
//Size of the Map
size_t os_getMapSize(const Heap* heap){
	return heap->sizeMap;
}

//Returns the Addr of the Map corresponding to the Useraddr NOT THE NIBBLE
MemAddr os_getMapAddr(const Heap *heap, MemAddr userAddr){
	return os_getMapStart(heap) + ((userAddr - os_getUseStart(heap))/2);
}
//Set a Vaule for the MapAddr corresponding to the UserAddr
void os_setMapAddrValue(const Heap *heap, MemAddr userAddr, MemValue value){
	MemAddr freeAddrMap = os_getMapAddr(heap,userAddr);
	// checks if high or low nibble 
	bool volatile highNible =((userAddr - os_getUseStart(heap))%2==0);
	if(highNible){
		setHighNibble(heap,freeAddrMap,value);
	}
	else{
		setLowNibble(heap,freeAddrMap,value);
	}
}
//Gets the Value of a Map Nibble for a UserAddr
MemValue os_getMapEntry (const Heap *heap, MemAddr userAddr){
	MemAddr mapAddr = os_getMapAddr(heap,userAddr);
	bool highNible =(userAddr - os_getUseStart(heap))%2==0;
	if(highNible){
		return getHighNibble(heap,mapAddr);
	}
	else{
		return getLowNibble(heap,mapAddr);
	}
}



/*
 * User Management
 */

size_t os_getUseSize(const Heap *heap){
	return heap->sizeUser;
}
MemAddr os_getUseStart(const Heap *heap){
	return heap->startaddrUse;
}


/*
 * Chunk Management
 */

//Dekrementiert die Addr solange bis die passende MapAddr gleich einer 0x0i fuer ein i:={1,..,7}
MemAddr os_getFirstByteOfChunk (const Heap *heap, MemAddr userAddr){
	while(os_getMapEntry(heap, userAddr) == 0x0F && userAddr>=os_getUseStart(heap)){
		userAddr -=1;
	}	
	return userAddr;
}
// Findet erstes Byte und zaehlt hoch solange die folgenden 0x0F sind Byte
uint16_t os_getChunkSize(const Heap *heap, MemAddr userAddr){
	MemAddr currentAddrChunk = os_getFirstByteOfChunk(heap,userAddr);
	currentAddrChunk +=1;
	uint16_t size = 0;
	MemValue valueOfcurrentAddr =os_getMapEntry(heap,currentAddrChunk);
	while(valueOfcurrentAddr == (MemValue)0x0F){
		size +=1;
		currentAddrChunk +=1;
		valueOfcurrentAddr =os_getMapEntry(heap,currentAddrChunk);
	}
	return size;
}




/*
 * Alloc Strat
 */

void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat){
	heap->currentStrat = allocStrat;
}


AllocStrategy os_getAllocationStrategy(const Heap *heap){
	AllocStrategy currentStrat= heap->currentStrat;
	return currentStrat;
}



/*
 * Malloc and Free
 */

MemAddr os_malloc(Heap* heap, uint16_t size){
	os_enterCriticalSection();
	//size anpassen
	
	
	// finden der ersten freien Addr im Userbereich
	MemAddr firstChunkAddrUser;
	switch (os_getAllocationStrategy(heap)){
		case OS_MEM_FIRST: firstChunkAddrUser = os_Memory_FirstFit(heap,size); break;
		default: firstChunkAddrUser= 0;
	}
	//falls keine frei ist
	if(firstChunkAddrUser==0){
		return 0;
	}
	//Map anpassen 
	os_setMapAddrValue(heap,firstChunkAddrUser,(MemValue)os_getCurrentProc());
	for (uint16_t i =1; i<size;i++){
		os_setMapAddrValue(heap,(firstChunkAddrUser + i),0xF);
	}
	
	os_leaveCriticalSection();
	return firstChunkAddrUser;
}
	
void os_free(Heap* heap, MemAddr addr){
	os_enterCriticalSection();
	MemAddr startOfChunk = os_getFirstByteOfChunk(heap,addr);
	uint16_t sizeOfChunk = os_getChunkSize(heap,addr);
	//versucht speicher von anderen Process frei zugeben
	MemValue ownerOfChunk =os_getMapEntry(heap,startOfChunk);
	if(ownerOfChunk != (MemValue)os_getCurrentProc()){
		os_error("os_free:not the right MemChunk");
	}
	else{
		os_setMapAddrValue(heap,startOfChunk,0);
		for (uint16_t i =1; i <= sizeOfChunk ; i++){
			os_setMapAddrValue(heap,(startOfChunk + i),0);
		}
	}
	os_leaveCriticalSection();
}


void os_freeProcessMemory (Heap *heap, ProcessID pid){
	os_enterCriticalSection();
	uint16_t index = 0;
	while(os_getMapEntry(heap,os_getUseStart(heap)+ index)!= pid && index < os_getUseSize(heap)){
		index +=1;
	}
	if(os_getMapEntry(heap,os_getUseStart(heap)+ index)== pid){
		os_free(heap,(os_getUseStart(heap)+ index));
	}
	os_leaveCriticalSection();
}


	






	
	




	
