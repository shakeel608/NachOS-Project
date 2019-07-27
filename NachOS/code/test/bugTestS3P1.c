#include "syscall.h"
int finshed_threads = 0;
int sync = 0;

void thread1(void* a)
{
	char *str = (char*) a;

	sync = 1;
	while(sync == 1);
	PutChar(*str);
	
	finshed_threads++;
	UserThreadExit();
}

void thread2(void* a)
{
	int *val = (int*) a;

	while(sync == 0);
	PutInt(*val);
	sync = 0;

	finshed_threads++;
	UserThreadExit();
}

int main()
{
    char arr = 'M';
    int num = 60;

	UserThreadCreate(thread1, &arr);

	UserThreadCreate(thread2, &num);

	while(finshed_threads < 2);
	Halt();
}