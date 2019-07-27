#include "syscall.h"

int
main() {
    int num;

    PutString("Enter -1 to quit\n");

    while (1) {
    	if(num == -1)
    		break;
    	else {
    	    GetInt(&num);
            PutInt(num);
            PutChar('\n');
         }
    }

    return 0;
}
