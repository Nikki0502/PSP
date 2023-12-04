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

typedef uint16_t MemAddr;

typedef uint8_t MemValue;

//Driver

typedef struct{
	MemAddr startAddr;
	MemAddr endAddr;
	MemAddr currAddr;
	uint16_t remainingBytesInSRAM;
	void (*init)();
	MemValue(*read)(MemAddr);
	void (*write)(MemAddr,MemValue);
	char name[];
}MemDriver;

//intSRAM Pointer
extern MemDriver intSRAM__;
#define intSRAM (&intSRAM__)


#endif /* OS_MEM_DRIVERS_H_ */