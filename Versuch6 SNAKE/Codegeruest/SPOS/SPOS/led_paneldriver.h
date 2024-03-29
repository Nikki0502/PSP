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

// Globales Led Matrix Array
// Der Framebuffer ist dann ein dreidimensionales Array: der erste Index selektiert die Ebene, der Zweite die Doppelzeile, der Dritte die Spalte.
// Eintrag in dem Array - - B2 G2 R2 B1 G1 R1
extern uint8_t frambuffer[2][16][32];


#endif
