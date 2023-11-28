/*
 * os_mem_drivers.c
 *
 * Created: 28.11.2023 16:07:00
 *  Author: nikum
 */ 
#include "os_mem_drivers.h"
#include "util.h"
#include "defines.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//Globale Var
MemDriver intSRAM ;

//Funktionnen des MemDriver-Types(init,read,wirte)