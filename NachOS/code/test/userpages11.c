#include "syscall.h"

void thread1(void* a)
{
	PutChar('0');
	PutChar('1');
	PutChar('2');
	PutChar('3');
	PutChar('4');
	PutChar('5');
	// PutChar('\n');

	UserThreadExit();
}

void thread2(void* a)
{
	PutChar('&');
	PutChar('{');
	PutChar('[');
	PutChar('|');
	PutChar(']');
	PutChar('}');
	// PutChar('\n');

	UserThreadExit();
}

int main()
{
	int tid1;
	tid1 = UserThreadCreate(thread1, 0);
	if (tid1 < 0 )
		Halt();

	int tid2;
	tid2 = UserThreadCreate(thread2, 0);
	if (tid2 < 0 )
		Halt();

	PutChar('S');

	//Halt();
}