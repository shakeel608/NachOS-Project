#include "copyright.h"
#include "system.h"

typedef struct ProcArgs
{
	int space;
	int procnum;	
}ProcArgs_t;

extern int do_ForkExec(int s);
extern void do_UserWaitPid(int pid);
