/*! \file
 *  \brief Scheduling library for the OS.
 *
 *  Contains the scheduling strategies.
 *
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2013
 *  \version  2.0
 */

#ifndef _OS_SCHEDULING_STRATEGIES_H
#define _OS_SCHEDULING_STRATEGIES_H

#include "defines.h"
#include "os_scheduler.h"
#include "os_memory.h"

//! Structure used to store specific scheduling informations such as a time slice
// This is a presence task

//! Used to reset the SchedulingInfo for one process
void os_resetProcessSchedulingInformation(ProcessID id);

//! Used to reset the SchedulingInfo for a strategy
void os_resetSchedulingInformation(SchedulingStrategy strategy);

//! Even strategy
ProcessID os_Scheduler_Even(const Process processes[], ProcessID current);

//! Random strategy
ProcessID os_Scheduler_Random(const Process processes[], ProcessID current);

//! RoundRobin strategy
ProcessID os_Scheduler_RoundRobin(const Process processes[], ProcessID current);

//! InactiveAging strategy
ProcessID os_Scheduler_InactiveAging(const Process processes[], ProcessID current);

//! RunToCompletion strategy
ProcessID os_Scheduler_RunToCompletion(const Process processes[], ProcessID current);

//! fuer RoundRobin
typedef struct{
	uint16_t timeslice;
	Age age[MAX_NUMBER_OF_PROCESSES]; 
}SchedulingInformation;

//! Warteschlange
typedef struct{
	ProcessID data[MAX_NUMBER_OF_PROCESSES];
	size_t size;
	uint8_t head;
	uint8_t tail;
	}ProcessQueue;

//! Funktionen fuer Benutzung der Warteschlange
uint8_t MLFQ_MapToQueue (Priority prio);
uint8_t MLFQ_getDefaultTimeslice (uint8_t queueID);
ProcessQueue * MLFQ_getQueue (uint8_t queueID);
void pqueue_init (ProcessQueue *queue);
void pqueue_reset (ProcessQueue *queue);
bool pqueue_hasNext (const ProcessQueue *queue);
ProcessID pqueue_getFirst (const ProcessQueue *queue);
void pqueue_dropFirst (ProcessQueue *queue);
void pqueue_append (ProcessQueue *queue, ProcessID pid);
void pqueue_removePID (ProcessQueue *queue, ProcessID pid);
void os_initSchedulingInformation ();
void MLFQ_removePID (ProcessID pid);
ProcessID os_Scheduler_MLFQ (const Process processes[], ProcessID current);
#endif
