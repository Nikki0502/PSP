/*
 * os_spi.h
 *
 * Created: 13.12.2023 11:12:50
 *  Author: nicki
 */ 


#ifndef OS_SPI_H_
#define OS_SPI_H_

void os_spi_int(void);

uint8_t os_spi_send(uint8_t data);

uint8_t os_spi_receive();



#endif /* OS_SPI_H_ */