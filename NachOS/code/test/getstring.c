#include "syscall.h"

int
main() {
    int size = 100;
    char arr[size];


    GetString(arr, size); 
    PutChar('\n');
   
    PutString(arr);
        PutChar('\n');

    PutChar('\n');

 
    return 0;
}
