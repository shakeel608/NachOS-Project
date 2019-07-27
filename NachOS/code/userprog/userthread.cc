#include "copyright.h"
#include "userthread.h"
#include "synchconsole.h"



static void StartUserThread (int f){

	int i;

	//recover f, arg and sp
	threadArgs_t* func_and_arg = (threadArgs_t*) f;
	int function = func_and_arg->f;
	int argument = func_and_arg->arg;
	int sp = func_and_arg->sp;

	//Lets clear all the registers of the machine
	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister (i, 0);

	//write the arguments on the registers
	machine->WriteRegister (4, argument);	

    //PC to point function
    machine->WriteRegister (PCReg, function);
    machine->WriteRegister (NextPCReg, function+4);

    // Set the stack register 2 or 3 pages below the pointer to the main program
    machine->WriteRegister (StackReg, sp);

	DEBUG ('t', "UserThread calling funtion %d running.\n", function);
    machine->Run();

}

void do_UserSemInit(int semAddress, int semCounter)
{

	int value;
	machine->ReadMem(semAddress, 2, &value);
	printf ("The value of the id is %d\n", value);
	if(value == 0)
	{
		machine->WriteMem(semAddress, 2, currentThread->space->userSemCounter);
		value = currentThread->space->userSemCounter;
		currentThread->space->userSemCounter++;
		currentThread->space->userSemaphores[value] = new Semaphore("ThreadSem", semCounter);
	}
	// machine->WriteMem(semAddress, 2, 20);
	printf ("The value of the id is %d\n", value);
	// printf("the sem ID is %d, the semcounter is %d",semAddress,semCounter);

}
void do_UserSemP(int semAddress)
{
	int value;
	machine->ReadMem(semAddress, 2, &value);
	if(value == 0){
		printf ("Uninitialized semaphore");
		return;
	}
	currentThread->space->userSemaphores[value]->P();

}
void do_UserSemV(int semAddress)
{
	int value;
	machine->ReadMem(semAddress, 2, &value);
	if(value == 0){
		printf ("Uninitialized semaphore");
		return;
	}
	currentThread->space->userSemaphores[value]->V();

}
int do_UserThreadCreate(int f, int arg){

	if((currentThread->space->tidcounter+1)*ThreadStackSize >= UserStackSize){
		printf ("Not enough space to allocate stack for thread\n");
		return -1;
	}

	currentThread->space->lock_livethreads->P();
	currentThread->space->livethreads++;
	currentThread->space->lock_livethreads->V();

	currentThread->space->lock_tidcounter->P();

	currentThread->space->tidcounter++;
	int this_tid = currentThread->space->tidcounter;
	
	
	currentThread->space->activeThreads[this_tid] = currentThread->tid;
	currentThread->space->lock_tidcounter->V();
	unsigned int numPages = currentThread->space->GetNumPages();

	//Put arguments and function in a structure so they can be sent to fork()
	threadArgs_t *func_and_arg = new threadArgs_t;
	func_and_arg->f = f;
	func_and_arg->arg = arg;
	func_and_arg->sp = numPages*PageSize - this_tid*ThreadStackSize-8;

	//Create the thread
    Thread *t = new Thread ("User thread");
    t->Fork (StartUserThread, (int)(func_and_arg));
	t->tid = this_tid;
	
	DEBUG ('t', "UserThread calling funtion %d created.\n", f);

    return this_tid;
}

void do_UserThreadExit(){
	DEBUG ('t', "UserThread finished.\n");

	currentThread->space->lock_livethreads->P();
	currentThread->space->livethreads--;
	currentThread->space->lock_livethreads->V();

	int this_tid = currentThread->tid;
	currentThread->space->createdThreads[this_tid]->V();

	currentThread->Finish();	
}

void do_Exit(){
	//Check if all threads in this address space are done
	currentThread->space->lock_livethreads->P();
	while(currentThread->space->livethreads > 0)
	{
		currentThread->space->lock_livethreads->V();	
		currentThread->Yield();
		currentThread->space->lock_livethreads->P();
	}
	currentThread->space->lock_livethreads->V();

	//Signal in case a Process is waiting this Process to finish
	int this_pro = currentThread->space->pro;
	createdPro[this_pro]->V();

	//Check if this is the last process in the system
	lock_livepro->P();
	livepro--;
	if(livepro > 0){	//this is not the last process, so just free the resources
		lock_livepro->V();

		#ifndef FILESYS_STUB
		//Delete entries that belong to the ending process
		fileSystem->DeleteEntriesOfProcess(currentThread->space->pro); 
		#endif // NOT FILESYS_STUB
		
		currentThread->space->FreeFrames();	  //Free memory
		currentThread->Finish();
	}
	lock_livepro->V();

}

void do_UserThreadJoin(int arg){
	currentThread->space->createdThreads[arg]->P();
	currentThread->space->createdThreads[arg]->V();
}

