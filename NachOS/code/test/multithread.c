#include "syscall.h"

void thread1(void* a)
{
	int *val = (int*) a;
	PutInt(*val);
	UserThreadExit();
}

int main()
{
	int i;
	int val[6];
	for(i=0; i<6; i++){
		val[i] = i;
		UserThreadCreate(thread1, &val[i]);
	}
	//PutString("Process Ending\n");
}