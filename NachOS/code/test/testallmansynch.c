#include "syscall.h"
int sync = 0;

void thread1(void* a)
{
	char *str = (char*) a;

	PutString("This is Thread 1 in Process 2\n");
	sync = 1;
	while(sync == 1);
	PutString("In P2T1 -- The string is: ");
	PutString(str);
	PutChar('\n');
	sync = 1;
	while(sync == 1);
	PutString("In P2T1\n");
	sync = 1;
	while(sync == 1);
	PutString("Ending P2T1\n");
	sync = 1;
	
	UserThreadExit();
}

void thread2(void* a)
{
	int *val = (int*) a;

	while(sync == 0);
	PutString("This is Thread 2 in Process 2\n");
	sync = 0;
	while(sync == 0);
	PutString("In P2T2 Integer is: ");
	PutInt(*val);
	PutChar('\n');
	sync = 0;
	while(sync == 0);
	PutString("In P2T2\n");
	sync = 0;
	while(sync == 0);
	PutString("Ending P2T2\n");

	UserThreadExit();
}

int main()
{

	PutString("This is Process 2\n");

    char arr[10];
	PutString("Input a string: ");
    GetString(arr, 10);
    PutString("string: ");
    PutString(arr);
	PutChar('\n');

    int num;
    PutString("Input a number: ");
	GetInt(&num);
    PutString("Number: ");
	PutInt(num);
	PutChar('\n');

	int tid1;
	tid1 = UserThreadCreate(thread1, arr);
	if (tid1 < 0 )
		Halt();

	int tid2;
	tid2 = UserThreadCreate(thread2, &num);
	if (tid2 < 0 )
		Halt();

	Halt();
}