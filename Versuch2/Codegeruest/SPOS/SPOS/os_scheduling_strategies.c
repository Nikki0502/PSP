/*! \file

Scheduling strategies used by the Interrupt Service RoutineA from Timer 2 (in scheduler.c)
to determine which process may continue its execution next.

The file contains five strategies:
-even
-random
-round-robin
-inactive-aging
-run-to-completion
*/

#include "os_scheduling_strategies.h"
#include "os_core.h"
#include "defines.h"

#include <stdlib.h>

/*!
* glabele var fuer scheduleing ninfos 
*/
SchedulingInformation schedulingInfo;


/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for RoundRobin and InactiveAging
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 *  \param strategy  The strategy to reset information for
 */
void os_resetSchedulingInformation(SchedulingStrategy strategy) {
    // This is a presence task
	Process current = *os_getProcessSlot(os_getCurrentProc());
	if(strategy== OS_SS_ROUND_ROBIN){
		schedulingInfo.timeslice=current.priority;
	}
	if(strategy == OS_SS_INACTIVE_AGING){
		for(int i =0; i< MAX_NUMBER_OF_PROCESSES; i++){
			schedulingInfo.age[i]=0;
		}
	}
}

/*!
 *  Reset the scheduling information for a specific process slot
 *  This is necessary when a new process is started to clear out any
 *  leftover data from a process that previously occupied that slot
 *
 *  \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(ProcessID id) {
    // This is a presence task
	schedulingInfo.age[id]=0;
}

/*!
 *  This function implements the even strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
ProcessID os_Scheduler_Even(const Process processes[], ProcessID current) {
	uint16_t processInReady= 0;
	// um alle prozesse die ready sind heraus zu finden
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES;i++){
		if(processes[i].state== OS_PS_READY){
			processInReady+=1;
		}
	}
	//falls kein Prozess in Ready soll idle
	if(processInReady == 0){
		current = 0;
	}
	else{
		while(true){
			//kuckt sich den naechsten Process an
			current+=1;
			//ausser es ist der Letzte Prozess dann den 1ten
			if (current==MAX_NUMBER_OF_PROCESSES){
				current = 1;
			}
			//ob dieser Prozess ready ist wenn ja breche ab
			if(processes[current].state == OS_PS_READY){
				break;
			}
		}
	}
    return current;
}

/*!
 *  This function implements the random strategy. The next process is chosen based on
 *  the result of a pseudo random number generator.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the random strategy.
 */
ProcessID os_Scheduler_Random(const Process processes[], ProcessID current) {
	int randnumber = rand();
	uint16_t processInReady= 0;
	// um alle prozesse die ready sind heraus zu finden
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES;i++){
		if(processes[i].state== OS_PS_READY){
			processInReady+=1;
		}
	}
	// um nicht das array mehr mals durch gehen zu muesse modulo benutzen
	randnumber = randnumber % processInReady;
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES;i++){
		if(randnumber== 0){
			current = i;
		}
		if(processes[i].state== OS_PS_READY){
			randnumber-=1;
		}
	}	
	//fals kein prozess ready ist soll idle 
	if (processInReady == 0){
		current = 0;
	}
    return current;
}

/*!
 *  This function implements the round-robin strategy. In this strategy, process priorities
 *  are considered when choosing the next process. A process stays active as long its time slice
 *  does not reach zero. This time slice is initialized with the priority of each specific process
 *  and decremented each time this function is called. If the time slice reaches zero, the even
 *  strategy is used to determine the next process to run.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the round robin strategy.
 */
ProcessID os_Scheduler_RoundRobin(const Process processes[], ProcessID current) {
	uint16_t processInReady= 0;
	// um alle prozesse die ready sind heraus zu finden
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES;i++){
		if(processes[i].state== OS_PS_READY){
			processInReady+=1;
		}
	}
	//falls kein Prozess in Ready soll idle
	if(processInReady == 0){
		current = 0;
		return current;
	}
   
	schedulingInfo.timeslice --;
	if (schedulingInfo.timeslice==0){
		current = os_Scheduler_Even(processes,current);
		schedulingInfo.timeslice = processes[current].priority;
	}
    return current;
}

/*!
 *  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
 *  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
 *  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
 *  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
 *  is to be returned is reset to its priority.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the inactive-aging strategy.
 */
ProcessID os_Scheduler_InactiveAging(const Process processes[], ProcessID current) {
	uint16_t processInReady= 0;
	// um alle prozesse die ready sind heraus zu finden
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES;i++){
		if(processes[i].state== OS_PS_READY){
			processInReady+=1;
		}
	}
	//falls kein Prozess in Ready soll idle
	if(processInReady == 0){
		current = 0;
		return current;
	}
    // This is a presence task
	//schedulingInfo.age[current] += processes[current].priority;
	for (int i =1; i< MAX_NUMBER_OF_PROCESSES;i++){
		schedulingInfo.age[i] += processes[i].priority;
	}
	
	ProcessID oldest=1;
	for(int i =1; i<MAX_NUMBER_OF_PROCESSES;i++){
		//falls kleiner als mom aeltester 
		if(processes[i].state!= OS_PS_UNUSED){
			if(schedulingInfo.age[oldest]< schedulingInfo.age[i]){
				oldest = i;
			}
			//falls gleich alt
			if(schedulingInfo.age[oldest]== schedulingInfo.age[i]){
				//falls prio groesser
				if (processes[oldest].priority< processes[i].priority){
					oldest = i;
				}
				// falls prio glecih
				else if (processes[oldest].priority== processes[i].priority){
					//falls id kleiner als odlest 
					if (oldest>i){
						oldest = i;
					}
				}
			}
		}
	}
	/*
	for (int i =1; i< MAX_NUMBER_OF_PROCESSES;i++){
		schedulingInfo.age[i] += processes[i].priority;
		if(i==oldest){
			schedulingInfo.age[i]=0;
		}
	}
	*/
	schedulingInfo.age[oldest]=0;
    return oldest;
}

/*!
 *  This function realizes the run-to-completion strategy.
 *  As long as the process that has run before is still ready, it is returned again.
 *  If  it is not ready, the even strategy is used to determine the process to be returned
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the run-to-completion strategy.
 */
ProcessID os_Scheduler_RunToCompletion(const Process processes[], ProcessID current) {
    // This is a presence task
	uint16_t processInReady= 0;
	// um alle prozesse die ready sind heraus zu finden
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES;i++){
		if(processes[i].state== OS_PS_READY){
			processInReady+=1;
		}
	}
	//falls kein Prozess in Ready soll idle
	if(processInReady == 0){
		current = 0;
		return current;
	}
	
	if(processes[current].state == OS_PS_UNUSED){
		current = os_Scheduler_Even(processes,current);
	}
    return current;
}
