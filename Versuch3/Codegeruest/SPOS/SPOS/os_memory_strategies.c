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
	MemAddr current = os_getUseStart(heap);
	uint16_t index = 0;
	while(current<(os_getUseStart(heap)+os_getUseSize(heap))){
		if(heap->driver->read(current)==0){
			index +=1;
		}
		else{
			index = 0;
		}
		current +=1;
	}
	if(index==size){
		return (current - size);
	}
	return 0;
}