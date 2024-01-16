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
	uint16_t sizeOfChunk = 1;
	//versucht speicher von anderen Process frei zugeben
	MemValue ownerOfChunk =os_getMapEntry(heap,startOfChunk);
	if(ownerOfChunk>0x7){
		os_error("os_free:not a shared mem");
		return ;
	}
	if(ownerOfChunk != (MemValue)os_getCurrentProc()){
		os_error("os_free:not the right MemChunk");
		return;
	}
	
	os_setMapAddrValue(heap,(startOfChunk),0);
	while(os_getMapEntry(heap,startOfChunk+sizeOfChunk) == 0xF){
		os_setMapAddrValue(heap,(startOfChunk + sizeOfChunk),0);
		sizeOfChunk++;
	}
	
	// Optimierungs Frame
	if(startOfChunk == heap->allocFrameStart){
		for(uint16_t i = startOfChunk + sizeOfChunk; i < heap->allocFrameEnd; i++){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameStart = i;
				break;
			}
			if(i==heap->allocFrameEnd-1){
				//reset
				heap->allocFrameStart = os_getMapStart(heap)+os_getMapSize(heap);
				heap->allocFrameEnd = os_getMapStart(heap);
			}
		}
	}
	else if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
		for (uint16_t i = startOfChunk; i > heap->allocFrameStart; i--){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameEnd = i;
				break;
			}
			if(i==heap->allocFrameStart+1){
				//reset
				heap->allocFrameStart = os_getMapStart(heap)+os_getMapSize(heap);
				heap->allocFrameEnd = os_getMapStart(heap);
			}
		}
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
	//os_getUseStart(heap)+os_getUseSize(heap)
	MemAddr startOfChunk = 0;
	uint16_t sizeOfChunk = 1;
	MemAddr current = heap->allocFrameStart;
	while (current< heap->allocFrameEnd){
		if(os_getMapEntry(heap,current)== pid){
			sizeOfChunk = 1;
			startOfChunk = current;
			os_setMapAddrValue(heap,(startOfChunk),0);
			while(os_getMapEntry(heap,startOfChunk+sizeOfChunk) == 0xF){
				os_setMapAddrValue(heap,(startOfChunk + sizeOfChunk),0);
				sizeOfChunk++;
			}
			// Optimierungs Frame
			if(startOfChunk == heap->allocFrameStart){
				for(uint16_t i = startOfChunk + sizeOfChunk; i < heap->allocFrameEnd; i++){
					if(os_getMapEntry(heap,i)!=0){
						heap->allocFrameStart = i;
						current = i-1;
						break;
					}
					if(i==heap->allocFrameEnd-1){
						//reset
						heap->allocFrameStart = os_getMapStart(heap)+os_getMapSize(heap);
						heap->allocFrameEnd = os_getMapStart(heap);
						current = heap->allocFrameEnd;
					}
				}
			}
			else if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
				for (uint16_t i = startOfChunk; i > heap->allocFrameStart; i--){
					if(os_getMapEntry(heap,i)!=0){
						heap->allocFrameEnd = i;
						break;
					}
					if(i==heap->allocFrameStart+1){
						//reset
						heap->allocFrameStart = os_getMapStart(heap)+os_getMapSize(heap);
						heap->allocFrameEnd = os_getMapStart(heap);
					}
				}
				current = heap->allocFrameEnd;
			}
		}
		current +=1;
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


/************************************************************************/
/*                          Shared Memory                               */
/************************************************************************/

/*
Zum Protokoll fuer die Allok.Tabelle
Es muss nicht erkennbar sein wem der Shared Mem gehört
Es muss erkennbar sein ob irgend ein Lesezugriff oder mehrere ausgefuehrt werden
Gleichzeiteges lesen vom mind. 2 Processen
Dagestellt in 4 bits(0,...,7 und F bis jetzt)
Idee:
Ein Shared Mem wird standart als 10 dagestellt
liesst ein Process wird dies um 1 erhoeht 
so waere bei 2 lesenden processen 12 an der Stelle(chunkLeader)
und sollte ein Process schreiben wollen muss dieser
warten bis der Chunkleader wieder 10 ist um dann diesen 
auf 9 zu setzen.
Somit kann man dann klar zwichen Lesen und Schreiben unterscheiden
Close muss dann wenn der Chunkleader zb 13 ist(3 Processe lesen) 
diesen um 1 wieder veringern und bei 9(max. 1er am schreiben) diesen wieder auf 10 setzen 
=> max 4 Processe gleichzeitig lesen, 1 gleichzeitig schreiben und erkennbar ob Processe lesen oder schreiben 
benutzen 9,10,11,12,13,14
*/

/*
Allocates a chunk of memory on the medium given by the driver and reserves it as shared memory.

Parameters
heap	The heap to be used.
size	The amount of memory to be allocated in Bytes. Must be able to handle a single byte and values greater than 255.

Returns
A pointer to the first Byte of the allocated chunk.
0 if allocation fails (0 is never a valid address).
*/
MemAddr os_sh_malloc (Heap *heap, size_t size){
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
	// Map als Schared Mem kennzeichnen
	os_setMapAddrValue(heap,firstChunkAddrUser,0xA);
	for (uint16_t i =1; i<size;i++){
		os_setMapAddrValue(heap,(firstChunkAddrUser + i),0xF);
	}
	
	os_leaveCriticalSection();
	return firstChunkAddrUser;
}

/*
Frees a chunk of shared allocated memory on the given heap

Parameters
heap	The heap to be used.
addr	An address inside of the chunk (not necessarily the start).
*/	
void os_sh_free (Heap *heap, MemAddr *addr){
	os_enterCriticalSection();
	MemAddr startOfChunk = os_getFirstByteOfChunk(heap,*addr);
	uint16_t sizeOfChunk = os_getChunkSize(heap,*addr);
	//versucht speicher von anderen Process frei zugeben
	MemValue ownerOfChunk =os_getMapEntry(heap,startOfChunk);
	if(ownerOfChunk<=0x7){
		os_error("os_sh_free:not a shared mem");
		os_leaveCriticalSection();
		return ;
	}
	while(ownerOfChunk != 0x10){
		os_yield();
	}
	/*
	if(ownerOfChunk != (MemValue)os_getCurrentProc()){
		os_error("os_sh_free:notTheRightMemChunk");
	}
	
	else{
		for (uint16_t i =0; i < sizeOfChunk ; i++){
			os_setMapAddrValue(heap,(startOfChunk + i),0);
		}
	}
	*/
	for (uint16_t i =0; i < sizeOfChunk ; i++){
		os_setMapAddrValue(heap,(startOfChunk + i),0);
	}
	// Optimierungs Frame
	if(startOfChunk == heap->allocFrameStart){
		for(uint16_t i = startOfChunk + sizeOfChunk; i < heap->allocFrameEnd; i++){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameStart = i;
				break;
			}
			if(i==heap->allocFrameEnd-1){
				//reset
				heap->allocFrameStart = os_getMapStart(heap)+os_getMapSize(heap);
				heap->allocFrameEnd = os_getMapStart(heap);
			}
		}
	}
	else if(startOfChunk+sizeOfChunk == heap->allocFrameEnd){
		for (uint16_t i = startOfChunk; i > heap->allocFrameStart; i--){
			if(os_getMapEntry(heap,i)!=0){
				heap->allocFrameEnd = i;
				break;
			}
			if(i==heap->allocFrameStart+1){
				//reset
				heap->allocFrameStart = os_getMapStart(heap)+os_getMapSize(heap);
				heap->allocFrameEnd = os_getMapStart(heap);
			}
		}
	}
	os_leaveCriticalSection();
}
	
/*
Opens a chunk of shared memory to prepare a reading process

Parameters
heap	The heap to be used
ptr	Pointer to an address of the chunk

Returns
MemAddr is the dereferenced argument ptr after opening the chunk
*/	
MemAddr os_sh_readOpen (const Heap *heap, const MemAddr *ptr){
	os_enterCriticalSection();
	MemAddr ptrAddr = os_getFirstByteOfChunk(heap,*ptr);
	MemAddr dereferencedPrt = *ptr;
	// falls auf privaten Mem aufgerugen
	if(os_getMapEntry(heap,os_getFirstByteOfChunk(heap,*ptr)) < 0x9){
		os_error("ReadOpen kein SH_MEM");
		os_leaveCriticalSection();
		return dereferencedPrt;
	}
	//gebe Rechenzeit ab, falls auf Shared Memory geschrieben wird oder bereits 4 Prozesse lesen
	volatile MemValue test = os_getMapEntry(heap,ptrAddr);
	while(os_getMapEntry(heap,ptrAddr) == 0x9 ||test == 0xE){
		os_yield();
	}
    os_setMapAddrValue(heap,ptrAddr,os_getMapEntry(heap,ptrAddr)+1);
	os_leaveCriticalSection();
	return dereferencedPrt;
}
	
/*
Opens a chunk of shared memory to prepare a writing process

Parameters
heap	The heap to be used
ptr	Pointer to an address of the chunk

Returns
MemAddr is the dereferenced argument ptr after opening the chunk
*/		
MemAddr os_sh_writeOpen (const Heap *heap, const MemAddr *ptr){
	os_enterCriticalSection();
	MemAddr ptrAddr = os_getFirstByteOfChunk(heap,*ptr);
	MemAddr dereferencedPrt = *ptr;
	// falls auf privaten Mem aufgerugen
	if(os_getMapEntry(heap,os_getFirstByteOfChunk(heap,*ptr)) < 0x9){
		os_error("WriteOpen kein SH_MEM");
		os_leaveCriticalSection();
		return dereferencedPrt;
	}
	// falls nicht bereit ist zu schreiben
	while(os_getMapEntry(heap,ptrAddr) != 0xA){
		os_yield();
	}
	// oeffnen fuer schreiben
	os_setMapAddrValue(heap,ptrAddr,0x9);
	os_leaveCriticalSection();
	return dereferencedPrt;
}
	
/*
Closes a chunk of shared memory to end an arbitrary access

Parameters
heap	The heap to be used
addr	Address of the chunk (not the first one)
*/		
void os_sh_close (const Heap *heap, MemAddr addr){
	os_enterCriticalSection();
	MemAddr firstByteOfChunkt =os_getFirstByteOfChunk(heap,addr);
	if (os_getMapEntry(heap,firstByteOfChunkt) > 10)
	{
		os_setMapAddrValue(heap,firstByteOfChunkt,os_getMapEntry(heap,firstByteOfChunkt)-1);
	}
	if (os_getMapEntry(heap,firstByteOfChunkt) == 9)
	{
		os_setMapAddrValue(heap,firstByteOfChunkt,0xA);
	}
	os_leaveCriticalSection();
}
	
/*
Function used to write onto shared memory

Parameters
heap	The heap to be used
ptr	Pointer to an address of the chunk to write to
offset	An offset that refers to the beginning of the chunk
dataSrc	Source of the data (this is always on internal SRAM)
length	Specifies how many bytes of data shall be written
*/		
void os_sh_write (const Heap *heap, const MemAddr *ptr, uint16_t offset, const MemValue *dataSrc, uint16_t length){
	if(os_getChunkSize(heap,*ptr)<(offset+length)){
		os_error("Zurif auserhalp eine gemeinse Speicheeerberaisch");
		return ;
	}	
	MemAddr derefPtr = os_sh_writeOpen(heap,ptr);
	MemAddr startOfChunk = os_getFirstByteOfChunk(heap,derefPtr);
	for(uint16_t i = 0; i<length;i++){
		heap->driver->write(startOfChunk+offset+i,heap->driver->read(((MemAddr)dataSrc+i)));
	}
	os_sh_close(heap,startOfChunk);
}
	
/*
Function used to read from shared memory

Parameters
heap	The heap to be used
ptr	Pointer to an address of the chunk to read from
offset	An offset that refers to the beginning of the chunk
dataDest	Destination where the data shall be copied to (this is always on internal SRAM)
length	Specifies how many bytes of data shall be read
*/		
void os_sh_read (const Heap *heap, const MemAddr *ptr, uint16_t offset, MemValue *dataDest, uint16_t length){
	if(os_getChunkSize(heap,*ptr)<(offset+length) ){
		os_error("Zurif auserhalp eine gemeinse Speicheeerberaisch");
		return;
	}
	MemAddr derefPtr = os_sh_readOpen(heap,ptr);
	MemAddr startOfChunk = os_getFirstByteOfChunk(heap,derefPtr);
	for(uint16_t i = 0; i<length;i++){
		heap->driver->write((MemAddr)dataDest+i,heap->driver->read(startOfChunk+i+offset));
	}
	os_sh_close(heap,startOfChunk);
}




	






	
	




	
