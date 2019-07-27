// addrspace.cc 
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option 
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

#include <strings.h>		/* for bzero */

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the 
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader * noffH)
{
    noffH->noffMagic = WordToHost (noffH->noffMagic);
    noffH->code.size = WordToHost (noffH->code.size);
    noffH->code.virtualAddr = WordToHost (noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost (noffH->code.inFileAddr);
    noffH->initData.size = WordToHost (noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost (noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost (noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost (noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
	WordToHost (noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost (noffH->uninitData.inFileAddr);
}



static ExceptionType
DoTranslatiton(int virtAddr, int* physAddr, TranslationEntry *pageTable, unsigned int pageTableSize)
{
    unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG('a', "\tTranslate 0x%x, %s: ", virtAddr, "write");
    

    // calculate the virtual page number, and offset within the page,
    // from the virtual address
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    
    if (vpn >= pageTableSize) {
        DEBUG('a', "virtual page # %d too large for page table size %d!\n", virtAddr, pageTableSize);
        return AddressErrorException;
    } else if (!pageTable[vpn].valid) {
        DEBUG('a', "virtual page # %d is not valid!\n", virtAddr, pageTableSize);
        return PageFaultException;
    }
    entry = &pageTable[vpn];
    pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong! 
    // An invalid translation was loaded into the page table or TLB. 
    if (pageFrame >= NumPhysPages) { 
        DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
        return BusErrorException;
    }
    entry->use = TRUE;      // set the use, dirty bits
    entry->dirty = TRUE;
    *physAddr = pageFrame * PageSize + offset;
    ASSERT((*physAddr >= 0) && ((*physAddr + 1) <= MemorySize));
    DEBUG('a', "phys addr = 0x%x\n", *physAddr);
    return NoException;
}



static bool
WriteMachineMem(int addr, int value, TranslationEntry *pageTable, unsigned int pageTableSize)
{
    ExceptionType exception;
    int physicalAddress;
     
    DEBUG('a', "Writing VA 0x%x, value 0x%x\n", addr, value);

    exception = DoTranslatiton(addr, &physicalAddress, pageTable, pageTableSize);
    if (exception != NoException) {
        machine->RaiseException(exception, addr);
        return FALSE;
    }
    machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
    
    return TRUE;
}



//----------------------------------------------------------------------
// ReadAtVirtual
//      Write to the virtal address space
//----------------------------------------------------------------------
static void 
ReadAtVirtual(  OpenFile *executable, 
                int virtualaddr, 
                int numBytes,
                int position,
                TranslationEntry *pageTable,
                unsigned pageTableSize){

    char tempBuffer[numBytes];
    int i;

    //machine->pageTable = pageTable;
    //machine->pageTableSize = numPages;

    executable->ReadAt(tempBuffer, numBytes, position);

    for(i = 0; i < numBytes; i++){
        WriteMachineMem(virtualaddr + i, tempBuffer[i], pageTable, pageTableSize);   
    }   
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical 
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace (OpenFile * executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
	(WordToHost (noffH.noffMagic) == NOFFMAGIC))
	SwapHeader (&noffH);
    ASSERT (noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	// we need to increase the size
    // to leave room for the stack
    numPages = divRoundUp (size, PageSize);
    size = numPages * PageSize;

    ASSERT (numPages <= NumPhysPages);	// check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG ('a', "Initializing address space, num pages %d, size %d\n",
	   numPages, size);

// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++)
      {
	  pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	  pageTable[i].physicalPage = frameProvider->GetEmptyFrame(AS_ORDERED);
	  pageTable[i].valid = TRUE;
	  pageTable[i].use = FALSE;
	  pageTable[i].dirty = FALSE;
	  pageTable[i].readOnly = FALSE;	// if the code segment was entirely on 
	  // a separate page, we could set its 
	  // pages to be read-only
      }


// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    //bzero (machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0)
      {
	  DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
		 noffH.code.virtualAddr, noffH.code.size);
      ReadAtVirtual(executable, 
                noffH.code.virtualAddr, 
                noffH.code.size,
                noffH.code.inFileAddr,
                pageTable,
                numPages);
	  //executable->ReadAt (&(machine->mainMemory[noffH.code.virtualAddr]),
		//	      noffH.code.size, noffH.code.inFileAddr);
      }
    if (noffH.initData.size > 0)
      {
	  DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
		 noffH.initData.virtualAddr, noffH.initData.size);
      ReadAtVirtual(executable, 
                noffH.initData.virtualAddr, 
                noffH.initData.size,
                noffH.initData.inFileAddr,
                pageTable,
                numPages);
	  //executable->ReadAt (&
	//		      (machine->mainMemory
	//		       [noffH.initData.virtualAddr]),
	//		      noffH.initData.size, noffH.initData.inFileAddr);
      }

    userSemCounter = 1;
    tidcounter = 0;
    livethreads = 0;
    lock_tidcounter = new Semaphore("lock_tidcounter",1);
    lock_livethreads = new Semaphore("lock_livethreads",1);

    for(i=0; i < MaxNumThreads; i++){
        createdThreads[i] = new Semaphore("ThreadSem", 0);
        activeThreads[i] = 0;
    }   


    

    interthread_lock->P();
    pro = procounter;
    interthread_lock->V();

    for (i = 0; i < numPages; i++){
        DEBUG ('p', "virtual Page %d, PhysicalPage %d\n", i, pageTable[i].physicalPage);
    }

    #ifndef FILESYS_STUB
    //Init Open Files Table
    openFilesTable = new ProcessOpenFilesTableEntry[MaxOpenFilesInProcess];
    for (i = 0; i < MaxOpenFilesInProcess; i++){
        openFilesTable[i].inUse = FALSE;
        openFilesTable[i].openFile = NULL;
        openFilesTable[i].fileName[0] = '\0';
    }
    #endif // NOT FILESYS_STUB

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace ()
{
  // LB: Missing [] for delete
  // delete pageTable;
  delete [] pageTable;
  // End of modification
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters ()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister (i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister (PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister (NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister (StackReg, numPages * PageSize - 16);
    DEBUG ('a', "Initializing stack register to %d\n",
	   numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void
AddrSpace::SaveState ()
{
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void
AddrSpace::RestoreState ()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}



//----------------------------------------------------------------------
// AddrSpace::GetNumPages
//      Return the numPages
//----------------------------------------------------------------------

unsigned int
AddrSpace::GetNumPages ()
{
    return numPages;
}


//----------------------------------------------------------------------
// AddrSpace::FreeFrames
//      Deallocate Memory
//----------------------------------------------------------------------

void
AddrSpace::FreeFrames ()
{
    unsigned int i;

    for (i = 0; i < numPages; i++)
    {
        frameProvider->ReleaseFrame(pageTable[i].physicalPage);
    }
}


#ifndef FILESYS_STUB
//  Look up file name in openFilesTable, and return the index in the table
//  Return -1 if the name isn't  in the table
//  "name" -- the file name to look up
int
AddrSpace::FindOpenFileIndex(const char *name)
{
    for (int i = 0; i < MaxOpenFilesInProcess; i++)
        if (openFilesTable[i].inUse && !strncmp(openFilesTable[i].fileName, name, CompleteFileNameMaxLen))
        return i;
    return -1;      // name not in table
}

//Add an entry to the openFilesTable
//Returns the index of the entry or -1 if there is no space
int
AddrSpace::AddToOpenFilesTable(const char *name, OpenFile *openFile)
{ 
    for (int i = 0; i < MaxOpenFilesInProcess; i++){
        if (!openFilesTable[i].inUse) {
            openFilesTable[i].inUse = TRUE;
            openFilesTable[i].openFile = openFile;
            strncpy(openFilesTable[i].fileName, name, CompleteFileNameMaxLen);
            return i;
        }
    }
    return -1;   // no space.
}

#endif // NOT FILESYS_STUB
