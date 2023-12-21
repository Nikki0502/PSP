/*
 * os_memory.c
 *
 * Created: 28.11.2023 16:12:59
 *  Author: nikum
 */ 
#include "os_memory.h"
#include "os_scheduler.h"
#include "os_core.h"
#include "os_memory_strategies.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>



/*
 * Nible Management
 */
void setLowNibble (const Heap *heap, MemAddr addr, MemValue value){
	if(addr < heap->startaddrUse && addr >= heap->startaddrMap){
		heap->driver->write(addr, ((getHighNibble(heap,addr)<<4) | value));
	}
}

void setHighNibble (const Heap *heap, MemAddr addr, MemValue value){
	if(addr < heap->startaddrUse && addr >= heap->startaddrMap){
		heap->driver->write(addr, ((getLowNibble(heap,addr) | (value<<4))));
	}
}

MemValue getLowNibble (const Heap *heap, MemAddr addr){
	MemValue volatile value =(heap->driver->read(addr) & 0b00001111);
	return value;
}

MemValue getHighNibble (const Heap *heap, MemAddr addr){
	MemValue value = (heap->driver->read(addr)>>4);
	return value;
}




/*
 *	Map Management
 */

//Start of the Map
MemAddr os_getMapStart(const Heap* heap){
	return heap->startaddrMap;
}
//Size of the Map
size_t os_getMapSize(const Heap* heap){
	return heap->sizeMap;
}

//Returns the Addr of the Map corresponding to the Useraddr NOT THE NIBBLE
MemAddr os_getMapAddr(const Heap *heap, MemAddr userAddr){
	return os_getMapStart(heap) + ((userAddr - os_getUseStart(heap))/2);
}
//Set a Vaule for the MapAddr corresponding to the UserAddr
void os_setMapAddrValue(const Heap *heap, MemAddr userAddr, MemValue value){
	MemAddr freeAddrMap = os_getMapAddr(heap,userAddr);
	// checks if high or low nibble 
	bool volatile highNible =((userAddr - os_getUseStart(heap))%2==0);
	if(highNible){
		setHighNibble(heap,freeAddrMap,value);
	}
	else{
		setLowNibble(heap,freeAddrMap,value);
	}
}
//Gets the Value of a Map Nibble for a UserAddr
MemValue os_getMapEntry (const Heap *heap, MemAddr userAddr){
	MemAddr mapAddr = os_getMapAddr(heap,userAddr);
	bool highNible =(userAddr - os_getUseStart(heap))%2==0;
	if(highNible){
		return getHighNibble(heap,mapAddr);
	}
	else{
		return getLowNibble(heap,mapAddr);
	}
}



/*
 * User Management
 */

size_t os_getUseSize(const Heap *heap){
	return heap->sizeUser;
}
MemAddr os_getUseStart(const Heap *heap){
	return heap->startaddrUse;
}


/*
 * Chunk Management
 */

//Gibt die erste Adresse eines allozierten Speicherblocks zurück
MemAddr os_getFirstByteOfChunk (const Heap *heap, MemAddr userAddr){
	while(os_getMapEntry(heap, userAddr) == 0x0F && userAddr>=os_getUseStart(heap)){
		userAddr -=1;
	}	
	return userAddr;
}
//Gibt die größte eines alloziierten Speicherblocks als uint16 zurück
uint16_t os_getChunkSize(const Heap *heap, MemAddr userAddr){
	MemAddr currentAddrChunk = os_getFirstByteOfChunk(heap,userAddr);
	currentAddrChunk +=1;
	uint16_t size = 0;
	MemValue valueOfcurrentAddr =os_getMapEntry(heap,currentAddrChunk);
	while(valueOfcurrentAddr == (MemValue)0x0F){
		size +=1;
		currentAddrChunk +=1;
		valueOfcurrentAddr =os_getMapEntry(heap,currentAddrChunk);
	}
	return size+1;
}




/*
 * Alloc Strat
 */

void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat){
	heap->currentStrat = allocStrat;
}


AllocStrategy os_getAllocationStrategy(const Heap *heap){
	AllocStrategy currentStrat= heap->currentStrat;
	return currentStrat;
}



/*
 * Malloc and Free
 */

/*Alloziiert Speicherplatz für einen Prozess
  returnt 0 wenn kein Speicherblock gefunden wurde, sonst erste Adresse von gefundenem Block */
MemAddr os_malloc(Heap* heap, uint16_t size){
	os_enterCriticalSection();

	/* Je nach Schedulingstrategie wird die erste Adresse des zu 
	 verwendenen freien Speicherblocks zurückgegeben */
	MemAddr firstChunkAddrUser=0;
	AllocStrategy current =os_getAllocationStrategy(heap);
	switch (current){
		case OS_MEM_FIRST: firstChunkAddrUser = os_Memory_FirstFit(heap,size,os_getUseStart(heap)); break;
		case OS_MEM_WORST: firstChunkAddrUser = os_Memory_WorstFit(heap,size); break;
		case OS_MEM_BEST: firstChunkAddrUser = os_Memory_BestFit(heap,size);   break;
		case OS_MEM_NEXT: firstChunkAddrUser = os_Memory_NextFit(heap,size);   break;
	}
	// next alloc leader zu weisen wenn Next Fit
	if(current == OS_MEM_NEXT){
		if(firstChunkAddrUser!=0){
			heap->lastAllocLeader=firstChunkAddrUser+size;
		}
		else{
			heap->lastAllocLeader=firstChunkAddrUser;
		}
	}
	//falls kein Speicherblock gefunden werden konnte
	// koennte zu problemem fuehren bei extHEAP
	if(firstChunkAddrUser == 0){
		//Start von letzten allozierten Bereich
		os_leaveCriticalSection();
		return 0;
	}
	// Optimierungs Frame
	if(firstChunkAddrUser< heap->allocFrameStart){
		heap->allocFrameStart = firstChunkAddrUser;
	}
	if(firstChunkAddrUser+size> heap->allocFrameEnd){
		heap->allocFrameEnd = firstChunkAddrUser+size;
	}
	//In der Map die entsprechenden Adressen des Speicherblocks für den Prozess reservieren
	os_setMapAddrValue(heap,firstChunkAddrUser,(MemValue)os_getCurrentProc());
	for (uint16_t i =1; i<size;i++){
		os_setMapAddrValue(heap,(firstChunkAddrUser + i),0xF);
	}
	
	os_leaveCriticalSection();
	return firstChunkAddrUser;
}
// Gibt Speicherplatz, der einem Prozess alloziiert wurde frei
void os_free(Heap* heap, MemAddr addr){
	//os_enterCriticalSection();
	MemAddr startOfChunk = os_getFirstByteOfChunk(heap,addr);
	uint16_t sizeOfChunk = os_getChunkSize(heap,addr);
	//versucht speicher von anderen Process frei zugeben
	MemValue ownerOfChunk =os_getMapEntry(heap,startOfChunk);
	if(ownerOfChunk != (MemValue)os_getCurrentProc()){
		os_error("os_free:not the right MemChunk");
	}
	else{
		for (uint16_t i =0; i < sizeOfChunk ; i++){
			os_setMapAddrValue(heap,(startOfChunk + i),0);
		}
	}
	// Optimierungs Frame
	if(startOfChunk == heap->allocFrameStart){
		for(uint16_t i = startOfChunk + sizeOfChunk; i <= heap->allocFrameEnd; i++){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameStart = i;
				break;
			}
		}
		/*
		if(startOfChunk == heap->allocFrameStart){
			heap->allocFrameStart = os_getUseStart(heap);
		}
		*/
	}
	else if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
		for (uint16_t i = startOfChunk; i >= heap->allocFrameStart; i--){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameEnd = i;
				break;
			}
		}
		/*
		if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
			heap->allocFrameStart = os_getUseStart(heap)+os_getUseSize(heap);
		}
		*/
	}
	//os_leaveCriticalSection();
}

/*
This function realises the garbage collection. When called, every allocated memory chunk of the given process is freed

Parameters
heap	The heap on which we look for allocated memory
pid	The ProcessID of the process that owns all the memory to be freed
*/
void os_freeProcessMemory (Heap *heap, ProcessID pid){
	//os_enterCriticalSection();
	MemAddr startOfChunk = 0;
	uint16_t sizeOfChunk = 0;
	MemAddr current = heap->allocFrameStart;
	while (current< heap->allocFrameEnd){
		if(os_getMapEntry(heap,current)== pid){
			startOfChunk = current;
			sizeOfChunk = os_getChunkSize(heap,startOfChunk);
			os_setMapAddrValue(heap,startOfChunk,0);
			for (uint16_t i =1; i < sizeOfChunk ; i++){
				os_setMapAddrValue(heap,(startOfChunk + i),0);
			}
		}
		current +=1;
	}
	// Optimierungs Frame
	if(startOfChunk == heap->allocFrameStart){
		for(uint16_t i = startOfChunk + sizeOfChunk; i <= heap->allocFrameEnd; i++){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameStart = i;
				break;
			}
		}
		/*
		if(startOfChunk == heap->allocFrameStart){
			heap->allocFrameStart = os_getUseStart(heap);
		}
		*/
	}
	else if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
		for (uint16_t i = startOfChunk; i >= heap->allocFrameStart; i--){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameEnd = i;
				break;
			}
		}
		/*
		if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
			heap->allocFrameStart = os_getUseStart(heap)+os_getUseSize(heap);
		}
		*/
	}
	//os_leaveCriticalSection();
}

/*
This will move one Chunk to a new location , To provide this the content of the old one is copied to the new location, 
as well as all Map Entries are set properly since this is a helper function for reallocation, it only works if the new Chunk is bigger than the old one.
Parameters
heap	The heap where the Moving takes place
oldChunk	The first Address of the old Chunk that is to be moved
oldSize	The size of the old Chunk
newChunk	The first Address of the new Chunk
newSize	The size of the new Chunk
*/
void moveChunk (Heap *heap, MemAddr oldChunk, size_t oldSize, MemAddr newChunk, size_t newSize){
	if (oldSize < newSize){
		os_setMapAddrValue(heap,newChunk,os_getCurrentProc());
		//Map verschieben
		for(uint16_t i = 1; i<newSize; i++){
			os_setMapAddrValue(heap,newChunk + i,0xF);
		}
		//Move Usebereich
		MemValue vorherigerWert;
		for(uint16_t i = 0; i<newSize;i++){
			vorherigerWert = heap->driver->read(oldChunk+i);
			heap->driver->write(newChunk+i, vorherigerWert);
		}
	}
}
/*
This is an efficient reallocation routine. It is used to resize an existing allocated chunk of memory. 
If possible, the position of the chunk remains the same. It is only searched for a completely new chunk if everything else does not fit For a more detailed description please use the exercise document.

Parameters
heap	The heap on which the reallocation is performed
addr	One adress inside of the chunk that is supposed to be reallocated
size	The size the new chunk is supposed to have

Returns
First adress (in use space) of the newly allocated chunk
*/
MemAddr os_realloc (Heap *heap, MemAddr addr, uint16_t size){
	os_enterCriticalSection();
	MemAddr current; //Hilfsvariable zum Zählen
	MemAddr reallocLeader = os_getFirstByteOfChunk(heap,addr);
	//Realloc Chunk gehört aktuellem Prozess nicht
	if(os_getMapEntry(heap,reallocLeader) != os_getCurrentProc()){
		os_error("Speicherbereich gehört diesem Prozess nicht");
		os_leaveCriticalSection();
		return 0;
	}
	
	size_t oldSize = os_getChunkSize(heap,reallocLeader);
	MemAddr freeChunkAfter = reallocLeader + oldSize; // erste Adresse des Bereichs nach dem Chunk
	size_t freeChunkAfterSize = 0;
	//Kleiner machen
	if(oldSize>size){
		for(uint16_t i = reallocLeader+size; i < reallocLeader+oldSize;i++){
			os_setMapAddrValue(heap,i,0x0);
		}
		os_leaveCriticalSection();
		return reallocLeader;
	}
	//Fall: Platz nach dem Chunk reicht aus
	if(os_getMapEntry(heap,freeChunkAfter)== 0){
		freeChunkAfterSize= os_getFreeChunkSize(heap,freeChunkAfter);
		//Reicht der Platz hinter dem Chunk?
		if( (freeChunkAfterSize + oldSize) >= size){
			for(uint16_t i = 0; i < size-oldSize; i++){
				os_setMapAddrValue(heap,(reallocLeader + oldSize)+i,0xF);
			}
			os_leaveCriticalSection();
			return reallocLeader;
		}
	}
	
	//Fall: Platz nach dem Chunk reicht nicht aus aber zsm mit Platz vor dem Chunk
	MemAddr currVorher = reallocLeader - 1 ;
	MemAddr leaderVorher;
	size_t sizeVorher = 0;
	//Größe des FreeChunk vor dem Chunk ermitteln
	while ( os_getMapEntry(heap,currVorher) == 0 ) {
		sizeVorher  += 1;
		currVorher --;
	}
	leaderVorher = reallocLeader - sizeVorher;
	//Reichen die Chunks vorne und hinten aus?
	if((sizeVorher + freeChunkAfterSize + oldSize) >= size){
		//Chunk so weit wie möglich nach Vorne verschieben
		moveChunk(heap,reallocLeader,oldSize, leaderVorher, size);
		//Nicht mehr benötigten Speicherplatz freigeben
		current = leaderVorher + size;
		while( os_getMapEntry(heap,current) == 0xF ) {
			os_setMapAddrValue(heap,current,0);
			current++;
		}
		os_leaveCriticalSection();
		return leaderVorher;
	}
	
	//Fall: Platz nach dem Chunk und vor dem Chunk reichen nicht zusammen aus
	MemAddr firstAddrOfNewChunk = 0;
	//Neuen Platz finden
	switch (os_getAllocationStrategy(heap)){
		case OS_MEM_FIRST: firstAddrOfNewChunk = os_Memory_FirstFit(heap,size,os_getUseStart(heap)); break;
		case OS_MEM_WORST: firstAddrOfNewChunk = os_Memory_WorstFit(heap,size); break;
		case OS_MEM_BEST: firstAddrOfNewChunk = os_Memory_BestFit(heap,size); break;
		case OS_MEM_NEXT: firstAddrOfNewChunk = os_Memory_NextFit(heap,size); break;
	}
	if(firstAddrOfNewChunk == 0){
		os_leaveCriticalSection();
		return 0;
	}
	moveChunk(heap,reallocLeader,oldSize,firstAddrOfNewChunk,size);
	//alten Chunk komplett freigeben(free)
	os_free(heap,addr);
	os_leaveCriticalSection();
	return firstAddrOfNewChunk;
}



	






	
	




	
