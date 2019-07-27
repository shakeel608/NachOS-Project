#include "syscall.h"

//int counter = 0;
void func(void *p) {

    int *m = (int*) p;
    PutInt(*m);
    UserThreadExit();
}

int main() {

    int i;
    int num[15];

    PutString("Testing main running\n");
    sem_t firstSem;
    SemInit(&firstSem,0);
    SemP(&firstSem);
    
    for(i = 0 ; i < 1 ; i++){
      num[i] = i;
	     UserThreadCreate(func, (void*) &num[i]);
   }     
   Halt()	;
}
