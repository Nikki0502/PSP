/*
 * os_memory_strategies.h
 *
 * Created: 30.11.2023 09:11:59
 *  Author: va675332
 */ 


#ifndef OS_MEMORY_STRATEGIES_H_
#define OS_MEMORY_STRATEGIES_H_

#include "os_memory.h"

MemAddr os_Memory_FirstFit (Heap *heap, size_t size, MemAddr startAddr);
MemAddr os_Memory_NextFit (Heap *heap, size_t size);
MemAddr os_Memory_WorstFit (Heap *heap, size_t size);
MemAddr os_Memory_BestFit (Heap *heap, size_t size);

MemAddr os_getFirstByteOfFree(const Heap *heap, MemAddr userAddr);
uint16_t os_getFreeChunkSize(const Heap *heap, MemAddr userAddr);

#endif /* OS_MEMORY_STRATEGIES_H_ */