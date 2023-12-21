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

/*
First-fit strategy.

Parameters
heap	The heap in which we want to find a free chunk
size	The size of the desired free chunk
start   The addr where the strat starts User

Returns
The first address(User) of the found free chunk, or 0 if no chunk was found.
*/
MemAddr os_Memory_FirstFit (Heap *heap, size_t size, MemAddr start){
	MemAddr countedSize = 0;
	while (start < os_getUseStart(heap)+os_getUseSize(heap)){
		if(os_getMapEntry(heap,start)==0){
			countedSize += 1;
		}
		else{
			countedSize = 0;
		}
		start += 1;
		if(countedSize==size){
			return (start-size);
		}
	}
	return 0;
}
/*
Next-fit strategy.

Parameters
heap	The heap in which we want to find a free chunk
size	The size of the desired free chunk

Returns
The first address of the found free chunk. Returns 0, if no chunk was found.

*/	
MemAddr os_Memory_NextFit (Heap *heap, size_t size){ 
	if(heap->lastAllocLeader==os_getUseStart(heap)||heap->lastAllocLeader==0){
		return os_Memory_FirstFit(heap, size, os_getUseStart(heap));
	}
	else{
		MemAddr behindStart=0;
		behindStart = os_Memory_FirstFit(heap, size, heap->lastAllocLeader);
		if(behindStart==0){
			behindStart =  os_Memory_FirstFit(heap, size, os_getUseStart(heap));
		}
		return behindStart;
	}
}
/*
Worst-fit strategy.

Parameters
heap	The heap in which we want to find a free chunk
size	The size of the desired free chunk

Returns
The first address of the found free chunk. Returns 0, if no chunk was found.
*/
MemAddr os_Memory_WorstFit (Heap *heap, size_t size){
	MemAddr current = os_getUseStart(heap);
	MemAddr biggestChunkAddr = 0;
	uint16_t biggestChunkSize = 0;
	uint16_t currentChunkSize = 0;
	while(current < os_getUseStart(heap)+os_getUseSize(heap)){
		if(os_getMapEntry(heap,current)==0){
			currentChunkSize +=1;
		}
		else{
			currentChunkSize = 0;
		}
		current +=1;
		if(currentChunkSize>biggestChunkSize){
			biggestChunkSize=currentChunkSize;
			biggestChunkAddr = current - biggestChunkSize;
		}
	}
	if(biggestChunkSize>=size){
		return biggestChunkAddr;
	}
	return 0;
}
/*
Best-fit strategy.

Parameters
heap	The heap in which we want to find a free chunk
size	The size of the desired free chunk

Returns
The first address of the found free chunk. Returns 0, if no chunk was found.
*/
MemAddr os_Memory_BestFit (Heap *heap, size_t size){
	MemAddr current = os_getUseStart(heap);
	uint16_t currentChunkSize = 0;
	uint16_t smallestChunkSize = 0;
	MemAddr smallestChunkAddr = 0;
	while(current < os_getUseStart(heap)+os_getUseSize(heap)){
		if(os_getMapEntry(heap,current)==0){
			
			currentChunkSize +=1;
		}
		else{
			if(currentChunkSize>=size){
				if(currentChunkSize<smallestChunkSize||smallestChunkSize==0){
					smallestChunkSize = currentChunkSize;
					smallestChunkAddr = current - smallestChunkSize;
				}
			}
			currentChunkSize = 0;
		}
		current+=1;
	}
	if(currentChunkSize>=size){
		if(currentChunkSize<smallestChunkSize||smallestChunkSize==0){
			smallestChunkSize = currentChunkSize;
			smallestChunkAddr = current - smallestChunkSize;
		}
	}
	if(smallestChunkSize>=size){
		return smallestChunkAddr;
	}
	return 0;
}

