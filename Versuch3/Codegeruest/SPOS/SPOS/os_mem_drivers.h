/*
 * os_mem_drivers.h
 *
 * Created: 28.11.2023 16:07:12
 *  Author: nikum
 */ 


#ifndef OS_MEM_DRIVERS_H_
#define OS_MEM_DRIVERS_H_

//Datentypen

typedef uint16_t MemAddr;

typedef uint8_t MemValue;

void init(void);

MemValue read(MemAddr addr);

void write(MemAddr addr , MemValue value);

typedef struct{
	MemAddr addr_start;
	MemAddr addr_end;
	void (*init)();
	MemValue(*read)(MemAddr);
	void (*write)(MemAddr,MemValue);
	char name[];
	MemAddr addr_curr;
}MemDriver;

#define intSRAM *intSRAM__


#endif /* OS_MEM_DRIVERS_H_ */