
#include "syscall.h"

void print(int ch, int n)
{
    int i;
    for (i = 0; i < n; i++) {

        PutInt(ch+i);
        PutChar('\n');
    }

    PutChar('\n');
}

int main()
{

    print(1,10);

    //Halt();

      return 0;
}