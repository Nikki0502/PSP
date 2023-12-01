/*
 * os_memory_strategies.h
 *
 * Created: 30.11.2023 09:11:59
 *  Author: va675332
 */ 


#ifndef OS_MEMORY_STRATEGIES_H_
#define OS_MEMORY_STRATEGIES_H_
#include "defines.h"
#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"
#include "atmega644constants.h"
#include "util.h"
#include "os_memory_strategies.h"
#include "os_memory.h"

MemAddr os_Memory_FirstFit (Heap *heap, size_t size);



#endif /* OS_MEMORY_STRATEGIES_H_ */