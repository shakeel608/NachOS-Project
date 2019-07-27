#include "syscall.h"


int
main() {


    char ch;

    
    PutChar('\n');

    PutString("Enter q to exit\n");

    while (1) {
        if(ch =='q') 
            break;
        else {
            ch = GetChar();
            PutChar(ch);
        }
  }

    return 0;
}
