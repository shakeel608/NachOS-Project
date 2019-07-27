#include "syscall.h"

int main()
{
	int i;
	int pid1;
	for(i = 0; i < 20; i++){
		pid1 = ForkExec("multithread");
		if(pid1 >= 0) UserWaitPid(pid1);
		PutString("\nProcess ");
		PutInt(i);
		PutString(" Done\n");
	}
}