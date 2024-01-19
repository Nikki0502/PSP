/*! \file
 *  \brief The driver for the RFID extension board
 *
 *  All necessary functions to access the RFID reader are present here.
 *
 *  \author   Lehrstuhl für Informatik 11 - RWTH Aachen
 *  \date     2009
 *  \version  0.8e
 *  \ingroup  project_group
 */

#ifndef _OS_RFIDDRIVER_H
#define _OS_RFIDDRIVER_H

#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

//! Initializes the RFID
void rfid_init();

//! Receives one RFID-Tag. This is the only one external called function
uint64_t rfid_receive(void);

#endif
