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
	//falls kein Prozess in Ready soll idle au�er ein Prozess hat den state OS_PS_BLOCKED
	if(processInReady == 0){
		if(processes[current].state != OS_PS_BLOCKED){
			current =0;
	    }
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
	//falls kein Prozess in Ready soll idle au�er ein Prozess hat den state OS_PS_BLOCKED
	if (processInReady == 0){
		if(processes[current].state != OS_PS_BLOCKED){
			current =0;
		}
	}
	else{
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
	//falls kein Prozess in Ready soll idle au�er der Prozess hat hat den state OS_PS_BLOCKED
	if(processInReady == 0){
		if(processes[current].state != OS_PS_BLOCKED){
			current = 0;
			return current;
		}
	}
	
	schedulingInfo.timeslice --;
	//Falls der Prozess blocked ist timeslice auf 0 setzen da dieser nicht ausgew�hlt werden soll
	if(processes[current].state == OS_PS_BLOCKED){
		schedulingInfo.timeslice = 0;
	}
	//test fuer Terminieren ob entweder Quantum leer oder hat sich selbst gekillt 
	if (schedulingInfo.timeslice==0 || processes[current].state==OS_PS_UNUSED){
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
	//falls kein Prozess in Ready soll idle au�er der Prozess hat hat den state OS_PS_BLOCKED
	if(processInReady == 0){
		if(processes[current].state!=OS_PS_BLOCKED){
			current = 0;
			return current;
		}
		else{
			schedulingInfo.age[current]=0;
			return current;
		}
	}
    // This is a presence task
	//schedulingInfo.age[current] += processes[current].priority;
	for (int i =1; i< MAX_NUMBER_OF_PROCESSES;i++){
		schedulingInfo.age[i] += processes[i].priority;
	}
	ProcessID oldest=1;
	for(int i =1; i<MAX_NUMBER_OF_PROCESSES;i++){
		//falls kleiner als mom aeltester 
		if(processes[i].state != OS_PS_UNUSED && processes[i].state != OS_PS_BLOCKED){
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
	//falls kein Prozess in Ready soll idle au�er der Prozess hat hat den state OS_PS_BLOCKED
	if(processInReady == 0){
		if(processes[current].state != OS_PS_BLOCKED){
			current = 0;
			return current;
		}
		return current;
	}
	
	if(processes[current].state == OS_PS_UNUSED){
		current = os_Scheduler_Even(processes,current);
	}
    return current;
}


/************************************************************************/
/*                             MLFQ                                     */
/************************************************************************/

/*
Maps a process-priority to a priority class.

Parameters
prio	The process-priority.

Returns
The index of the ProcessQueue/priority class
*/
uint8_t MLFQ_MapToQueue (Priority prio){
	return 0;
}

/*
Returns the default number of timeslices for a specific ProcessQueue/priority class.

Parameters
queueID	The index of the ProcessQueue/the priority class.

Returns
Number of timeslices.
*/
uint8_t MLFQ_getDefaultTimeslice (uint8_t queueID){
	return 0;
}
	
/*
Returns the corresponding ProcessQueue.

Returns a pointer to the ProcessQueue with index queueID from the schedulingInformation

Parameters
queueID	Index of the queue.

Returns
Pointer to the specific ProcessQueue.
*/
ProcessQueue* MLFQ_getQueue (uint8_t queueID){
	return 0;
}

/* 
 Initializes the given ProcessQueue with a predefined size
 Parameters
 queue The ProcessQueue to initialize.
*/ 
void pqueue_init ( ProcessQueue *queue){
	//Eingabepuffer initialisieren
	//Gr��te des Puffers festlegen
	queue->size = MAX_NUMBER_OF_PROCESSES;
	//Head und Tail auf 0 (leere Warteschlange)
	queue->head = 0;
	queue->tail = 0;
}
/*
Resets the given ProcessQueue.

Parameters
queue	The ProcessQueue to reset.
*/
void pqueue_reset (ProcessQueue *queue){
	queue->head = 0;
	queue->tail = 0;
}

/*
Checks whether there is next a ProcessID.

Parameters
queue	The ProcessQueue to check.

Returns
True if queue has a next element
*/
bool pqueue_hasNext (const ProcessQueue *queue){
	//Wenn queue leer return false
	if(queue->tail == queue->head){
		return false;
	}
	//sonst true
	else{
		return true;
	}
	
}

/*
Returns the first ProcessID of the given ProcessQueue.

Parameters
queue	The specific ProcessQueue.

Returns
the first ProcessID.
*/
ProcessID pqueue_getFirst (const ProcessQueue *queue){
	return queue->data[queue->tail];
}

/*
Drops the first ProcessID of the given ProcessQueue.

Parameters
queue	The specific ProcessQueue.
*/
void pqueue_dropFirst (ProcessQueue *queue){
	queue->data[queue->tail] = 0;
	queue->tail++;
}

/*
Appends a ProcessID to the given ProcessQueue.

Parameters
queue	The ProcessQueue in which the pid should be appended.
pid	The ProcessId to append.
*/
void pqueue_append (ProcessQueue *queue, ProcessID pid){
	queue->data[queue->head] = pid;
	queue->head++;
}

/*
Removes a ProcessID from the given ProcessQueue.

Parameters
queue	The ProcessQueue from which the pid should be removed.
pid	The ProcessId to remove.
*/
void pqueue_removePID (ProcessQueue *queue, ProcessID pid){
	uint8_t current = queue->tail;
	while(current != queue->head){
		if(queue->data[current] == pid){
			queue->data[current] = 0;
			uint8_t help = current+1;
			while(help != queue->head){
				queue->data[current] = queue->data[help];
				current ++;
				help++;
			}
			queue->data[queue->head] = 0;
			queue->head--;
		}
		
	}
}

/*
Reset the scheduling information for a specific process slot This is necessary when a new process is started to clear out any leftover data from a process that previously occupied that slot

Parameters
id	The process slot to erase state for
*/
void os_initSchedulingInformation (){}

/*
Function that removes the given ProcessID from the ProcessQueues.

Parameters
pid	The ProcessId to remove.
*/
void MLFQ_removePID (ProcessID pid){}

/*
This function implements the multilevel-feedback-queue with 4 priority-classes. 
Every process is inserted into a queue of a priority-class and gets a default amount of timeslices which are class dependent.
If a process has no timeslices left, it is moved to the next class. If a process yields, it is moved to the end of the queue.

Parameters
processes	An array holding the processes to choose the next process from.
current	The id of the current process.

Returns
The next process to be executed determined on the basis of the even strategy.
*/
ProcessID os_Scheduler_MLFQ (const Process processes[], ProcessID current){
	return 0;
}







