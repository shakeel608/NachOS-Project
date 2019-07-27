#include "syscall.h"

void thread1(void* a)
{
	PutChar('a');
	PutChar('b');
	PutChar('c');
	PutChar('d');
	PutChar('e');
	PutChar('f');
	// PutChar('\n');

	UserThreadExit();
}

void thread2(void* a)
{
	PutChar('A');
	PutChar('B');
	PutChar('C');
	PutChar('D');
	PutChar('E');
	PutChar('F');
	// PutChar('\n');

	UserThreadExit();
}

int main()
{
	int tid1;
	tid1 = UserThreadCreate(thread1, 0);
	if (tid1 < 0 )
		Halt();

	UserThreadJoin(tid1);
	
	int tid2;
	tid2 = UserThreadCreate(thread2, 0);
	if (tid2 < 0 )
		Halt();

	PutChar('J');
}