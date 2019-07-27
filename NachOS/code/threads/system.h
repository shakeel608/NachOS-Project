// system.h
//      All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

// Initialization and cleanup routines
extern void Initialize (int argc, char **argv);	// Initialization,
						// called before anything else
extern void Cleanup ();		// Cleanup, called when
						// Nachos is done.
extern Thread *currentThread;	// the thread holding the CPU
extern Thread *threadToBeDestroyed;	// the thread that just finished
extern Scheduler *scheduler;	// the ready list
extern Interrupt *interrupt;	// interrupt status
extern Statistics *stats;	// performance metrics
extern Timer *timer;		// the hardware alarm clock

#ifdef USER_PROGRAM

#include "machine.h"
extern Machine *machine;	// user program memory and registers
#include "synchconsole.h"
extern SynchConsole *synchconsole;
#include "frameprovider.h"
extern FrameProvider *frameProvider;
#define MAX_STRING_SIZE 256  //Local Buffer Size
#define MaxNumPro 256
extern int procounter;  //count the number of processes created
extern int livepro; //count the number of live processes
extern Semaphore *interthread_lock; //lock to protect sections between threads
extern Semaphore *lock_livepro; //lock to protect livepro counter
extern Semaphore *createdPro[MaxNumPro]; //Semaphores to keep track of created prcesses

#endif  //USER_PROGRAM

#ifdef FILESYS_NEEDED		// FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice *postOffice;
#endif

#endif // SYSTEM_H
