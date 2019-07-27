#include "copyright.h"
#include "forkexec.h"
#include "noff.h"

//int tidcounter = 0;

// Check if there is space to allocate the process
static int CheckPhysicalSpace (OpenFile * executable)
{
    NoffHeader noffH;
    executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);

    unsigned int size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;
    unsigned int nmPgs = divRoundUp (size, PageSize);
    unsigned int framesAvailabe = frameProvider->NumAvailFrame();
    
    if(framesAvailabe < nmPgs)
    	return -1;

    return 0;
}


static void StartForkedProcess (int s){

	ProcArgs_t* procargs = (ProcArgs_t*) s;
	AddrSpace *space = (AddrSpace*) procargs->space;
	int procnum = procargs->procnum;


    currentThread->space = space;
    currentThread->space->pro = procnum;

    space->InitRegisters ();	// set the initial register values
    space->RestoreState ();	// load page table register

    machine->Run ();		// jump to the user progam
    ASSERT (FALSE);		// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"

}


int do_ForkExec(int s){

	interthread_lock->P();
    procounter++;
    int this_pro = procounter;
    interthread_lock->V();

	ProcArgs_t *procargs = new ProcArgs_t;
	procargs->procnum = this_pro;

	int i;
	int value;
	int lenght;

	//get the lenght of the string
	i = 0;
	while(1){
		machine->ReadMem(s+i, 1, &value);
		if(value == 0)
			break;
		else
			i++;
	}
	lenght = i;

	char filename[lenght+1];

	//get the string (executable name)
	for(i = 0; i < lenght; i++){
		machine->ReadMem(s+i, 1, &value);
		filename[i] = value;
	}
	filename[i] = 0x00;

    OpenFile *executable = fileSystem->Open (filename);
    AddrSpace *space;

    if (executable == NULL){
		printf ("Unable to open file %s\n", filename);
		interthread_lock->P();
    	procounter--;
	    interthread_lock->V();
		delete executable;
		return -1;
	}
	if(CheckPhysicalSpace (executable) < 0){
		printf ("Not enough space to run process %s\n", filename);
		interthread_lock->P();
    	procounter--;
	    interthread_lock->V();
		delete executable;
		return -1;
	}
	lock_livepro->P();
	livepro++;
	lock_livepro->V();

    space = new AddrSpace (executable);
    procargs->space = (int)space;

    delete executable;		// close file


	//Create the thread*/
    Thread *t = new Thread ("Process thread");
    t->Fork (StartForkedProcess, (int)(procargs));
    
    return this_pro;
}

void do_UserWaitPid(int pid){
	createdPro[pid]->P();
	createdPro[pid]->V();
}
