/*
 * os_memory_strategies.c
 *
 * Created: 30.11.2023 09:11:31
 *  Author: va675332
 */ 

#include "os_memory.h"
#include <stdbool.h>

//Gibt die erste Speicheradresse von einem freien Block zurück
MemAddr os_getFirstByteOfFree(const Heap *heap, MemAddr userAddr){
	while(os_getMapEntry(heap, userAddr) == 0x00 && userAddr>=os_getUseStart(heap)){
		userAddr -=1;
	}
	return userAddr+1;
}

//Gibt die größte eines freien Speicherblocks zurück
uint16_t os_getFreeChunkSize(const Heap *heap, MemAddr userAddr){
	MemAddr currentAddrFree = os_getFirstByteOfFree(heap,userAddr);
	uint16_t size = 0;
	MemValue valueOfcurrentAddr =os_getMapEntry(heap,currentAddrFree);
	while(valueOfcurrentAddr == (MemValue)0x0 && currentAddrFree < (os_getUseStart(heap) + os_getUseSize(heap) ) ) {
		size +=1;
		currentAddrFree +=1;
		valueOfcurrentAddr =os_getMapEntry(heap,currentAddrFree);
	}
	return size;
}

// Alloc Strats

//First Fit
MemAddr os_Memory_FirstFit (Heap *heap, size_t size){	
	MemAddr current = os_getUseStart(heap);
	uint16_t index = 0;
	while(current<(os_getUseStart(heap)+os_getUseSize(heap))){
		if(os_getMapEntry(heap,current)==0){
			index +=1;
		}
		else{
			index = 0;
		}
		current +=1;
		if(index==size){
			return (current - size);
		}
		
	}
	return 0;
}

MemAddr os_Memory_NextFit (Heap *heap, size_t size){
	return 0;
}
//Wählt den größten freien Speicherblock aus, der groß genug ist
MemAddr os_Memory_WorstFit (Heap *heap, size_t size){
	MemAddr current = os_getUseStart(heap);
	MemAddr biggestChunkLeader = 0;
	size_t biggestSize = 0;
	while(current  <   (os_getUseStart(heap) + os_getUseSize(heap))   ){
	    if(os_getMapEntry(heap,current) == 0){	
			if(biggestSize < os_getFreeChunkSize(heap, current)){
				biggestSize = os_getFreeChunkSize(heap,current);
				biggestChunkLeader = os_getFirstByteOfFree(heap,current);
				current += biggestSize;
			}
		}
		current +=1;
	} 
	if(size <= biggestSize){
		return biggestChunkLeader;
	}
	return 0;
}
//Wäht den kleinsten freien Speicherblock aus, der groß genug ist
MemAddr os_Memory_BestFit (Heap *heap, size_t size){
	MemAddr current = os_getUseStart(heap);
	MemAddr bestChunkLeader = 0;
	size_t bestSize =0;
	bool found = false;
	while(current < (os_getUseStart(heap) + os_getUseSize(heap))){
		if(os_getMapEntry(heap,current) == 0){
			if(os_getFreeChunkSize(heap,current) == size){
				return os_getFirstByteOfFree(heap,current);
			}
			if(bestChunkLeader != 0){
				if(os_getFreeChunkSize(heap,current) < bestSize && os_getFreeChunkSize(heap,current) >= size){
					bestChunkLeader = os_getFirstByteOfFree(heap,current);
					bestSize = os_getFreeChunkSize(heap,current);
					current += bestSize;
				}
			}
			else{
				if(os_getFreeChunkSize(heap,current) >= size ){
					bestChunkLeader = os_getFirstByteOfFree(heap,current);
					bestSize = os_getFreeChunkSize(heap,current);
					current += bestSize;
				}
			}
		}
		current += 1;
	}
		return bestChunkLeader;
}


