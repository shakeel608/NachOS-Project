#include "syscall.h"

int main()
{
	cd("myDir");
	PutString("Parent\n");
	ForkExec("userpgs1");
}