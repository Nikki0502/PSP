/*
 * os_memheap_drivers.c
 *
 * Created: 28.11.2023 16:11:32
 *  Author: nikum
 */ 

#include "os_memheap_drivers.h"
#include "util.h"
#include "defines.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//Globale Var
Heap intHeap__;

//Funktionen
void os_initHeaps(void){}
uint8_t os_getHeapListLenght(void){}
Heap* os_lookupHeap(uint8_t index){}
