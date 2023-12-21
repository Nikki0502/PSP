/*
 * os_mem_drivers.h
 *
 * Created: 28.11.2023 16:07:12
 *  Author: nikum
 */ 


#ifndef OS_MEM_DRIVERS_H_
#define OS_MEM_DRIVERS_H_

#include <stdint.h>

//Datentypen
typedef uint16_t MemAddr; //Adresse auf SRAM
typedef uint8_t MemValue; //Wwet der Adresse
//Driver
typedef struct{
	MemAddr startAddr;
	MemAddr endAddr;
	void (*init)(); //initialisierung des Speichermediums
	MemValue(*read)(MemAddr); //Wert den MemAddr referenziert auslesen
	void (*write)(MemAddr,MemValue); //Wer den MemAddr referenziert ändern
	//char name[];
}MemDriver;
//intSRAM Pointer 
MemDriver intSRAM__;
#define intSRAM (&intSRAM__)

MemDriver extSRAM__;
#define extSRAM (&extSRAM__)

#endif /* OS_MEM_DRIVERS_H_ */