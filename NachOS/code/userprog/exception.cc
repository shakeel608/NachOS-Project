// exception.cc
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "userthread.h"
#include "forkexec.h"
#include "dofilesys.h"


//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------

static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}


//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions
//      are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler (ExceptionType which)
{
    int type = machine->ReadRegister (2);




    if(which == SyscallException)
    {
      switch(type)
      {


        case SC_Halt:
        {
          interrupt->Halt ();
          break;
        }
        case SC_Exit:
        {
          do_Exit();
          interrupt->Halt ();
          break;
        }

        case SC_PutChar:
        {
          char c;
          c = (char) machine->ReadRegister (4); //int to char conversion
          synchconsole->SynchPutChar(c);  //value in registers is in int asciii
          break;
        }

        case SC_PutString:
        { 

          DEBUG('c', "PutString system call  initiated  by user program.\n");
          char *to_buf = new char[MAX_STRING_SIZE];
          int from;
          from = machine->ReadRegister(4); //Register 4 contains address of string
          synchconsole->copyStringFromMachine(from,to_buf,MAX_STRING_SIZE);
          synchconsole->SynchPutString(to_buf);//  synchconsole  SynchPutString is used
          delete [] to_buf;
          break;

        }
        case SC_GetChar:
        {
          int ch;
          ch = (int) synchconsole->SynchGetChar();   //(int at begining)
          machine->WriteRegister(2, ch);
          break;
        }
        case SC_GetString:
        {
          char *buf = new char[MAX_STRING_SIZE];
          int from;
          int size;

          from = machine->ReadRegister(4);  //register 4 contains address of the string
          size = machine->ReadRegister(5);  // register 5  contains size of string
          synchconsole->SynchGetString(buf, size);
          synchconsole->copyStringToMachine(buf, from, size); //copies a string from the MIPS mode to the Linux mode
          delete [] buf;
          break;
        }

        case SC_PutInt:
        {
          int val;
          val = machine->ReadRegister(4);
          synchconsole->SynchPutInt(val);
          break;

        }

        case SC_GetInt:
         {
           int val, till;

           till = machine->ReadRegister(4);
           synchconsole->SynchGetInt(&val);
           machine->WriteMem(till, sizeof(int), val);
          break;
        }

        case SC_UserThreadCreate:
        {
          //activeThreads
          int f = machine->ReadRegister (4);
          int arg = machine->ReadRegister (5);
          int r;
          r = do_UserThreadCreate(f, arg);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_UserThreadExit:
        {
          do_UserThreadExit();
          break;
        }

        case SC_UserThreadJoin:
        {
          int arg = machine->ReadRegister (4);
          do_UserThreadJoin(arg);
          break;
        }

        case SC_ForkExec:
        {
          int s = machine->ReadRegister (4);
          int r;
          r = do_ForkExec(s);
          machine->WriteRegister(2, r);
          break;
        }

        case SC_UserWaitPid:
        {
          int pid = machine->ReadRegister (4);
          do_UserWaitPid(pid);
          break;
        }
        case SC_cpunix:
        {
          int arg1 = machine->ReadRegister (4);
          int arg2 = machine->ReadRegister (5);
          do_cpunix(arg1, arg2);
          break;
        }
        case SC_filepr:
        {
          int arg = machine->ReadRegister (4);
          do_filepr(arg);
          break;
        }
        case SC_DIR:
        {
          do_DIR();
          break;
        }
        case SC_rm:
        {
          int arg = machine->ReadRegister (4);
          int r;
          r = do_rm(arg);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_ls:
        {
          do_ls();
          break;
        }
        case SC_mkdir:
        {
          int arg = machine->ReadRegister (4);
          int r;
          r = do_mkdir(arg);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_cd:
        {
          int arg = machine->ReadRegister (4);
          int r;
          r = do_cd(arg);
          machine->WriteRegister(2, r);
          break;
        }

        case SC_open:
        {
          int arg1 = machine->ReadRegister (4);
          int arg2 = machine->ReadRegister (5);
          int arg3 = machine->ReadRegister (6);
          int r;
          r = do_open(arg1, arg2, arg3);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_read:
        {
          int arg1 = machine->ReadRegister (4);
          int arg2 = machine->ReadRegister (5);
          int r;
          r = do_read(arg1, arg2);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_write:
        {
          int arg1 = machine->ReadRegister (4);
          int arg2 = machine->ReadRegister (5);
          int arg3 = machine->ReadRegister (6);
          int r;
          r = do_write(arg1, arg2, arg3);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_close:
        {
          int arg = machine->ReadRegister (4);
          int r;
          r = do_close(arg);
          machine->WriteRegister(2, r);
          break;
        }
        case SC_lseek:
        {
          int arg1 = machine->ReadRegister (4);
          int arg2 = machine->ReadRegister (5);
          int r;
          r = do_lseek(arg1, arg2);
          machine->WriteRegister(2, r);
          break;
        }


        case SC_SemInit:
        {  
          int semID = machine->ReadRegister (4);
          int semCounter = machine->ReadRegister (5);
          do_UserSemInit(semID,semCounter);
          break;
        }
        case SC_SemP:
        {  
          int semIDAddress = machine->ReadRegister (4);
          do_UserSemP(semIDAddress);
          break;
        }
        case SC_SemV:
        {  
          int semIDAddress = machine->ReadRegister (4);
          do_UserSemV(semIDAddress);
          break;
        }

        default:
        {
          printf ("Unexpected user mode exception %d %d\n", which, type);
          ASSERT (FALSE);
        }
      }
        UpdatePC ();

    }




    // LB: Do not forget to increment the pc before returning!
    // End of addition
}
