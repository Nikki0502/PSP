/*
 * os_memory.h
 *
 * Created: 28.11.2023 16:13:19
 *  Author: nikum
 */ 


#ifndef OS_MEMORY_H_
#define OS_MEMORY_H_

#include "defines.h"
#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"
#include "os_process.h"
#include <stdint.h>


/*
 * Nible Management
 */
void setLowNibble (const Heap *heap, MemAddr addr, MemValue value);
void setHighNibble (const Heap *heap, MemAddr addr, MemValue value);
MemValue getLowNibble (const Heap *heap, MemAddr addr);
MemValue getHighNibble (const Heap *heap, MemAddr addr);

/*
 *	Map Management
 */
size_t os_getMapSize(const Heap *heap);
MemAddr os_getMapStart(const Heap *heap);
MemAddr os_getMapAddr(const Heap *heap, MemAddr userAddr);
void os_setMapAddrValue(const Heap *heap, MemAddr userAddr, MemValue value);
MemValue os_getMapEntry (const Heap *heap, MemAddr userAddr);

/*
 * User Management
 */
size_t os_getUseSize(const Heap *heap);
MemAddr os_getUseStart(const Heap *heap);

/*
 * Chunk Management
 */
MemAddr os_getFirstByteOfChunk (const Heap *heap, MemAddr userAddr);
uint16_t os_getChunkSize(const Heap *heap, MemAddr userAddr);

/*
 * Alloc Strat
 */
void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat);
AllocStrategy os_getAllocationStrategy(const Heap *heap);


/*
 * Malloc and Free
 */
MemAddr os_malloc(Heap* heap, uint16_t size);
void os_free(Heap* heap, MemAddr addr);
void os_freeProcessMemory (Heap *heap, ProcessID pid);
void moveChunk (Heap *heap, MemAddr oldChunk, size_t oldSize, MemAddr newChunk, size_t newSize);
MemAddr os_realloc (Heap *heap, MemAddr addr, uint16_t size);

#endif /* OS_MEMORY_H_ */