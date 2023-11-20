/*! \file
 *  \brief Scheduling module for the OS.
 *
 * Contains everything needed to realise the scheduling between multiple processes.
 * Also contains functions to start the execution of programs.
 *
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2013
 *  \version  2.0
 */

#include "os_scheduler.h"

#include "lcd.h"
#include "os_core.h"
#include "os_input.h"
#include "os_scheduling_strategies.h"
#include "os_taskman.h"
#include "util.h"
#include "defines.h"


#include <avr/interrupt.h>
#include <stdbool.h>

//----------------------------------------------------------------------------
// Private Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

//! Array of states for every possible process
Process os_processes[MAX_NUMBER_OF_PROCESSES];

//! Index of process that is currently executed (default: idle)
ProcessID currentProc ; 

//----------------------------------------------------------------------------
// Private variables
//----------------------------------------------------------------------------

//! Currently active scheduling strategy
SchedulingStrategy currentSchedulingStrategy;

//! Count of currently nested critical sections
uint8_t verschachtelungsTiefe;

//----------------------------------------------------------------------------
// Private function declarations
//----------------------------------------------------------------------------

//! ISR for timer compare match (scheduler)
ISR(TIMER2_COMPA_vect)
__attribute__((naked));

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

/*!
 *  Timer interrupt that implements our scheduler. Execution of the running
 *  process is suspended and the context saved to the stack. Then the periphery
 *  is scanned for any input events. If everything is in order, the next process
 *  for execution is derived with an exchangeable strategy. Finally the
 *  scheduler restores the next process for execution and releases control over
 *  the processor to that process.
 */
ISR(TIMER2_COMPA_vect) {
//sichern des Laufzeitkontext
saveContext();

currentProc = os_getCurrentProc();
//sichern des des Stackpointes fuer den Processstack des aktuellen Processes
os_processes[currentProc].stackpointer.as_ptr = &SP;//rot unterstrichen soll hier klar gehen laut Doc

//Setzen des SP Reg auf den Scheduler Stack
BOTTOM_OF_ISR_STACK = &SP; //???

//Aufruf des des Taskman
if(os_getInput()==0b00001001){
	while(os_getInput()==0b00001001){}
	os_taskManMain();
}


//Setzen des Prozesszustandes des aktuellen Prozesses auf OS_PS_READY
os_processes[currentProc].state = OS_PS_READY;

//Auswahl des naechsten fortzusetzenden Prozesses durch Aufruf der aktuell verwendeten Schedulingstrategie
switch(os_getSchedulingStrategy()){
	case OS_SS_RANDOM : currentProc = os_Scheduler_Random(os_processes, currentProc);break;
	case OS_SS_EVEN : currentProc = os_Scheduler_Even(os_processes, currentProc);break;
	//keine lust das für die andern zu machen gerade und so macht das keine fehlermeldung
	default : currentProc = 0;break;
}

//Setzen des Prozesszustandes des fortzusetzenden Prozesses auf OS_PS_RUNNING
os_processes[currentProc].state = OS_PS_RUNNING;

//Wiederherstellen des Stackpointers für den Prozessstack des fortzusetzenden Prozesses
SP = os_processes[currentProc].stackpointer.as_ptr;

//Wiederherstellen des Laufzeitkontext und automatischer Ruecksprung
restoreContext();
}

/*!
 *  This is the idle program. The idle process owns all the memory
 *  and processor time no other process wants to have.
 */
void idle(void){
	//in endlossschleife "...." ausgeben 
	while (true){
		lcd_clear();
		lcd_writeProgString(PSTR("...."));
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

/*!
 *  This function is used to register the given program for execution.
 *  A stack will be provided if the process limit has not yet been reached.
 *  This function is multitasking safe. That means that programs can repost
 *  themselves, simulating TinyOS 2 scheduling (just kick off interrupts ;) ).
 *
 *  \param program  The function of the program to start.
 *  \param priority A priority ranging 0..255 for the new process:
 *                   - 0 means least favourable
 *                   - 255 means most favourable
 *                  Note that the priority may be ignored by certain scheduling
 *                  strategies.
 *  \return The index of the new process or INVALID_PROCESS as specified in
 *          defines.h on failure
 */
ProcessID os_exec(Program *program, Priority priority) {
	//finde den ersten freien platz in Array
	int first_unused_process = 0;
	for (int i = 0; i <= MAX_NUMBER_OF_PROCESSES;i++){
		//erster freier platz
		if (os_processes[i].state==OS_PS_UNUSED){
			first_unused_process = i;
			break;
		}
		//falls kein platz frei ist 
		if(i==MAX_NUMBER_OF_PROCESSES){
			return INVALID_PROCESS;
		}
	}
	//programmzeiger ueberpruefen
	if (*program==NULL){
		return INVALID_PROCESS;
	}
	//Programm, Prozesszustand und Prozesspriorität speichern
	os_processes[first_unused_process].state = OS_PS_READY;
	os_processes[first_unused_process].program = *program;//program
	os_processes[first_unused_process].priority = priority;
	os_processes[first_unused_process].id = first_unused_process;
	
	//Prozessstack vorbereiten
	//neuen Prozess definieren(einfacher zum tippen)
	Process newProcess = os_processes[first_unused_process];
	//Rücksprungadresse speichern und aufteilen in 2 Byte
    uint16_t processadress = newProcess.stackpointer.as_int;
	
	//ist das hier die richitge stelle?
	os_enterCriticalSection();
   
	newProcess.stackpointer.as_int = PROCESS_STACK_BOTTOM(newProcess.id);
	uint8_t lowbyte = (uint8_t)(processadress & 0xff);
	uint8_t highbyte = (uint8_t)(processadress >> 8) & 0xff;
	//Rücksprungadresse auf Stack speichern
	newProcess.stackpointer.as_ptr = &lowbyte;
	newProcess.stackpointer.as_int ++;
	newProcess.stackpointer.as_ptr = &highbyte;
	newProcess.stackpointer.as_int ++;
	//noch STACK_SIZE_PROC einbauen?
	
	for(int i=0 ; i<33 ; i++){
		newProcess.stackpointer.as_ptr = 0b00000000;
		newProcess.stackpointer.as_int ++;
	}
	
	os_leaveCriticalSection();
	
	return newProcess.id;
}

/*!
 *  If all processes have been registered for execution, the OS calls this
 *  function to start the idle program and the concurrent execution of the
 *  applications.
 */
void os_startScheduler(void) {
	// Setze var currentprocess auf 0(idle)
	currentProc = 0;
	// idle auf Running
	os_processes[currentProc].state = OS_PS_RUNNING;
	// Setzen des Stackpointers auf den Prozessstack des Leerlaufprozesse
	SP = os_processes[currentProc].stackpointer.as_ptr;
	//Sprung in den Leerlaufprozess mit restoreContext()
	restoreContext();
}

/*!
 *  In order for the Scheduler to work properly, it must have the chance to
 *  initialize its internal data-structures and register.
 */
void os_initScheduler(void){
	// Hier werden alle auszuführenden Programme mit dieser Funkt in autostart_head eingefühgt 
	//To DO:
	//Welche Programme sollen den da eingefühgt werden?
	REGISTER_AUTOSTART(program);
	
	// Init os_processes mit unused ps
	for(int i = 0; i <MAX_NUMBER_OF_PROCESSES; i++){
		os_processes[i].state = OS_PS_UNUSED;
	}
	
	//exec idle asl erstes sollte somit auch pid=0 haben
	os_exec(*idle,DEFAULT_PRIORITY);
	
	//solange in der Liste ein Element ist 
	while (autostart_head != NULL){
		//falls ein Programm zum auto starten markiet ist 
		if(true){
			os_exec(autostart_head->program, DEFAULT_PRIORITY);
		}
		// naestes programm in autostart_head
		autostart_head = autostart_head->next;
	}
}

/*!
 *  A simple getter for the slot of a specific process.
 *
 *  \param pid The processID of the process to be handled
 *  \return A pointer to the memory of the process at position pid in the os_processes array.
 */
Process *os_getProcessSlot(ProcessID pid) {
    return os_processes + pid;
}

/*!
 *  A simple getter to retrieve the currently active process.
 *
 *  \return The process id of the currently active process.
 */
ProcessID os_getCurrentProc(void) {
	
	//Kann sein das das einfacher geht idk
	
	
	// läuft ein mal über alle Processe drueber bis den gefunden der gerade lauft 
	for (int i = 0; i <= MAX_NUMBER_OF_PROCESSES;i++){
		//momentan laufender Prozess
		if (os_processes[i].state==OS_PS_RUNNING){
			currentProc = os_processes[i].id;
		}
    return currentProc;
}

/*!
 *  Sets the current scheduling strategy.
 *
 *  \param strategy The strategy that will be used after the function finishes.
 */
void os_setSchedulingStrategy(SchedulingStrategy strategy){
	currentSchedulingStrategy = strategy;
}

/*!
 *  This is a getter for retrieving the current scheduling strategy.
 *
 *  \return The current scheduling strategy.
 */
SchedulingStrategy os_getSchedulingStrategy(void) {
    return currentSchedulingStrategy;
}

/*!
 *  Enters a critical code section by disabling the scheduler if needed.
 *  This function stores the nesting depth of critical sections of the current
 *  process (e.g. if a function with a critical section is called from another
 *  critical section) to ensure correct behaviour when leaving the section.
 *  This function supports up to 255 nested critical sections.
 */
void os_enterCriticalSection(void) {
	if (verschachtelungsTiefe<=0){
		os_errorPStr("Zu oft Crit.Sec. verallsen");
	}
	//savve SREG
	char savedSERG = SREG;
	// deaktiviere Global Interupt Bit
	SREG &= 0b01111111;
	// inkrement verschahtelungstiefe
	verschachtelungsTiefe+=1;
	//deaktivieren des Schedulers "Bit OCIE2A im Register TIMSK2 auf den Wert 0 gesetz"
	TIMSK2 &= ~(1 << OCIE2A);
	SREG = savedSERG;
}

/*!
 *  Leaves a critical code section by enabling the scheduler if needed.
 *  This function utilizes the nesting depth of critical sections
 *  stored by os_enterCriticalSection to check if the scheduler
 *  has to be reactivated.
 */
void os_leaveCriticalSection(void){
	if (verschachtelungsTiefe<=0){
		os_errorPStr("Zu oft Crit.Sec. verallsen");
	}
	//savve SREG
	char savedSERG = SREG;
	// deaktiviere Global Interupt Bit
	SREG &= 0b01111111;
	// dekrement verschahtelungstiefe
	verschachtelungsTiefe-=1;
	//aktivieren des Schedulers wenn kein critSection mehr
	if(verschachtelungsTiefe==0){
		// Setze das Bit OCIE2A im Register TIMSK2 auf 1
		TIMSK2 |= (1 << OCIE2A);
	}
	SREG = savedSERG;
}

/*!
 *  Calculates the checksum of the stack for a certain process.
 *
 *  \param pid The ID of the process for which the stack's checksum has to be calculated.
 *  \return The checksum of the pid'th stack.
 */
StackChecksum os_getStackChecksum(ProcessID pid) {
#warning IMPLEMENT STH. HERE
    return 0;
}
