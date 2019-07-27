#include "syscall.h"
int finshed_threads = 0;
int sync = 0;

void thread1(void* a)
{
	char *str = (char*) a;

	sync = 1;
	while(sync == 1);
    PutString("Hello from thread1\n");
	PutString(str);
	PutChar('\n');
	
	finshed_threads++;
	UserThreadExit();
}

void thread2(void* a)
{
	char *str = (char*) a;

	while(sync == 0);
    PutString("Hello from thread2\n");
	PutString(str);
	PutChar('\n');
	sync = 0;

	finshed_threads++;
	UserThreadExit();
}

int main()
{
    char arr1[] = "Sent from Main to T1";
    char arr2[] = "For T2";

    PutString("Hello from Main Thread\n");

	UserThreadCreate(thread1, &arr1);

	UserThreadCreate(thread2, &arr2);

	while(finshed_threads < 2);
	Halt();
}