#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "utility.h"
#include "console.h"

class SynchConsole {
    public:
        SynchConsole(char *readFile, char *writeFile);
        // initialize the hardware console device
        ~SynchConsole(); // clean up console emulation
        void SynchPutChar(const char ch); // Unix putchar(3S)
        int SynchGetChar(); // Unix getchar(3S)
        void SynchPutString(const char *s); // Unix puts(3S)
        void SynchGetString(char *s, int n); // Unix fgets(3S)
        //Reads from Memory
       static  void copyStringFromMachine( int from, char *to, unsigned size);  
        // Writes To Memory
       static  void copyStringToMachine(char *from, int to, unsigned size);
        void SynchGetInt(int *n);
        void  SynchPutInt(int n); //writes a signed integer using the function snprint

        void threadCreateSem();
        void threadDestroySem();



    private:
        Console *console;
};

#endif // SYNCHCONSOLE_H


