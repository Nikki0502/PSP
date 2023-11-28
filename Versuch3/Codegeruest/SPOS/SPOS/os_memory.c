/*
 * os_memory.c
 *
 * Created: 28.11.2023 16:12:59
 *  Author: nikum
 */ 
#include "os_memory.h"
#include "util.h"
#include "defines.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//Funktionen
MemAddr os_malloc(Heap* heap, uint16_t size){}
	
void os_free(Heap* heap, MemAddr addr){}
	
size_t os_get{Map,Use}Size(Heap const* heap){}
	
MemAddr os_get{Map,Use}Size(Heap const* heap){}
	
uint16_t os_getChunkSize(Heap const* heap, MemAddr addr){}
	
void os_setAllocationStrategy(Heap* heap, AllocStrategy allocStrat){}
	
AllocStrategy os_getAllocationStrategy(Heap* const* heap){}
	
