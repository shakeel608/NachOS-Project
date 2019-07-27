// progtest.cc
//      Test routines for demonstrating that Nachos can load
//      a user program and execute it.
//
//      Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "synchconsole.h"
#include "addrspace.h"
#include "synch.h"

//----------------------------------------------------------------------
// StartProcess
//      Run a user program.  Open the executable, load it into
//      memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess (char *filename)
{
    OpenFile *executable = fileSystem->Open (filename);
    AddrSpace *space;

    if (executable == NULL)
      {
	  printf ("Unable to open file %s\n", filename);
	  return;
      }
    space = new AddrSpace (executable);
    currentThread->space = space;

    delete executable;		// close file

    space->InitRegisters ();	// set the initial register values
    space->RestoreState ();	// load page table register

    machine->Run ();		// jump to the user progam
    ASSERT (FALSE);		// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
//      Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void
ReadAvail (int arg)
{
    readAvail->V ();
}
static void
WriteDone (int arg)
{
    writeDone->V ();
}

//----------------------------------------------------------------------
// ConsoleTest
//      Test the console by echoing characters typed at the input onto
//      the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------
//#ifdef CHANGED
void
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console (in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore ("read avail", 0);
    writeDone = new Semaphore ("write done", 0);
    int flag = 0;
    for (;;)
      {
	  readAvail->P ();	// wait for character to arrive
	  ch = console->GetChar ();

      if(ch != EOF) {
        if(flag == 0) {
            console->PutChar('<');
            writeDone->P ();
            flag = 1;
        }
        if(flag ==1 && ch==10) {
            console->PutChar('>');
            writeDone->P ();
            flag = 0;
        }

        console->PutChar(ch); //Echo it
        writeDone->P ();  //Wait for write to finish
      }

      if(ch =='q' || ch == EOF) {

        return;  // if q, quit
      }

    }
}



void
SynchConsoleTest (char *in, char *out)
{

  char ch;
  //char inputstring[5];
  SynchConsole *synchconsoleLocal = new SynchConsole(in, out);
  //Testing few strings
  //synchconsoleLocal->SynchPutString("Wellcome to SynchConsoleTest\n");
  //synchconsoleLocal->SynchPutString("Write at least 5 characters\n");
  //synchconsoleLocal->SynchGetString(inputstring, 5);
  //synchconsoleLocal->SynchPutString("First 5 characters:\n");
  //synchconsoleLocal->SynchPutString(inputstring);
  //synchconsoleLocal->SynchPutString("\nRest of the string:\n");

  int flag = 0;
  for (;;)
    {
    //readAvail->P ();  // wait for character to arrive
    ch = synchconsoleLocal->SynchGetChar ();

    if(ch != EOF) {
      if(flag == 0) {
          synchconsoleLocal->SynchPutChar('<');
          //writeDone->P ();
          flag = 1;
      }


      if(flag ==1 && ch==10) {
          synchconsoleLocal->SynchPutChar('>');
          //writeDone->P ();
          flag = 0;
      }


      synchconsoleLocal->SynchPutChar(ch); //Echo it
      //writeDone->P ();  //Wait for write to finish
    }

    if(ch =='q') {
      synchconsoleLocal->SynchPutChar('>');
      //writeDone->P ();
      return;  // if q, quit
    }

  }


    /*char ch = 'c' ;
        delete synchconsole;
        synchconsole = new  SynchConsole (in, out);
        while ((ch = synchconsole-> SynchGetChar ()) != EOF) {
        synchconsole-> SynchPutChar ('<');
        synchconsole-> SynchPutChar (ch);
      }
      synchconsole-> SynchPutChar ('>');
        fprintf (stderr, " Solaris: EOF Detected in SynchConsole! \n " );
     */
}
