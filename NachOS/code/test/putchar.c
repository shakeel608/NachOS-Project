#include "syscall.h"

void print(char c, int n)
{
	int i;
	for (i = 0; i < n; i++) {

		PutChar(c+i);
	}

	PutChar('\n');
}

int main()
{

	print('a',26);

	//Halt();

 	return 1;
}
