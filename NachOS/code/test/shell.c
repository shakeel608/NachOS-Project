#include "syscall.h"

int
main ()
{
	int newProc;
    char buffer[40];

	while (1){
		PutString(">> ");
		GetString(buffer, 40); 

		if (buffer[0] != '\0')
		{
			newProc = ForkExec(buffer);
			if(newProc<0){
				continue;
			}
			UserWaitPid(newProc);
		}
	}
}
