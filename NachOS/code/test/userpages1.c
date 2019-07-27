#include "syscall.h"

int main()
{
	ForkExec("small1");
	ForkExec("small2");
	PutChar('Z');
}