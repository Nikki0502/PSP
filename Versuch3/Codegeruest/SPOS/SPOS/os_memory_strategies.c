/*
 * os_memory_strategies.c
 *
 * Created: 30.11.2023 09:11:31
 *  Author: va675332
 */ 

#include "os_memory.h"


// Alloc Strats

//First Fit
MemAddr os_Memory_FirstFit (Heap *heap, size_t size){
	MemAddr currentaddr = os_getMapStart(heap);
	uint8_t indexSize = 0;
	uint16_t addrDiffMap = 0;
	MemAddr addrUser = 0;
	while(currentaddr < os_getUseStart(heap)){
		// HighNiblle frei
		if(getHighNibble(heap,currentaddr)==0){
			indexSize +=1;
		}
		//platz nicht gross genug 
		else{
			indexSize = 0;
		}
		// ausreichender Platz gefunden und UserAddr calc
		if(indexSize == size){
			addrUser = os_getUseStart(heap) + (2*addrDiffMap);		
		}
		
		// LowNibble frei
		if(getLowNibble(heap,currentaddr) ==0 ){
			indexSize +=1;
		}
		//platz nicht gross genug 
		else{
			indexSize = 0;
		}
		// ausreichender Platz gefunden und UserAddr calc
		if(indexSize == size){
			addrUser = os_getUseStart(heap) + (2*addrDiffMap)+1;		
		}
		// naechste addr 
		addrDiffMap +=1;
		currentaddr +=1;
	}
	return addrUser;
}