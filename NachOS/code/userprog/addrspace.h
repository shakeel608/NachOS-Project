// addrspace.h 
//      Data structures to keep track of executing user programs 
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "synch.h"


#define UserStackSize		512	// increase this as necessary!
#define ThreadStackSize     64
#define MaxNumThreads       1024 //(512 : 64 )



class Semaphore;

#ifndef FILESYS_STUB
#define CompleteFileNameMaxLen 250
#define MaxOpenFilesInProcess 10
class ProcessOpenFilesTableEntry {
  public:
    bool inUse;
    OpenFile* openFile;
    char fileName[CompleteFileNameMaxLen + 1];
};
#endif // NOT FILESYS_STUB

typedef struct node
{
	int threadID;
  int active;
  int child[10];
  node *next;
  Semaphore *sem;
}threadList_t;


class AddrSpace
{
  public:
    AddrSpace (OpenFile * executable);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace ();		// De-allocate an address space

    void InitRegisters ();	// Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState ();		// Save/restore address space-specific
    void RestoreState ();	// info on a context switch 

    unsigned int GetNumPages (); // Get the number of pgs 
    void FreeFrames(); //Deallcate Memory

    #ifndef FILESYS_STUB
    int AddToOpenFilesTable(const char *name, OpenFile *openFile);
    int FindOpenFileIndex(const char *name);
    #endif // NOT FILESYS_STUB

  private:
      TranslationEntry * pageTable;	// Assume linear page table translation
    // for now!
    unsigned int numPages;	// Number of pages in the virtual 
    // address space

  public:
    int tidcounter;  //count the number of threads created
    int livethreads; //count the number of live threads
    Semaphore *lock_tidcounter; //lock to protect tidcounter
    Semaphore *lock_livethreads; //lock to protect livethreads
    Semaphore *createdThreads[MaxNumThreads]; //Semaphores to keep track of created threads
    Semaphore *userSemaphores[100];
    int userSemCounter;
    threadList_t *threads;
    int activeThreads[MaxNumThreads];
    int pro; //the proccess that this addrspace belongs
    #ifndef FILESYS_STUB
    ProcessOpenFilesTableEntry *openFilesTable;
    #endif // NOT FILESYS_STUB

};

#endif // ADDRSPACE_H
