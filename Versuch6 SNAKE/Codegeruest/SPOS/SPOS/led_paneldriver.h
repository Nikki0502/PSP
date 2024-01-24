/*! \file
 *  \brief Low level functions to draw premade things on the LED Panel
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2019
 */
#ifndef _LED_PANELDRIVER_H
#define _LED_PANELDRIVER_H
#include <avr/io.h>
#include <stdbool.h>


//! Initializes registers
void panel_init();

//! Starts interrupts
void panel_startTimer(void);

//! Stops interrupts
void panel_stopTimer(void);

//! Initalizes interrupt timer
void panel_initTimer(void);

uint8_t os_getFramebufferEntry(uint8_t ebene, uint8_t x, uint8_t y);
void os_setFramebufferEntry(uint8_t ebene, uint8_t x, uint8_t y, uint8_t value);

#endif
