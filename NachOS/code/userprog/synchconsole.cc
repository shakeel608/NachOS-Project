#ifdef CHANGED
#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;
//static Semaphore *readConsoleLock;
//static Semaphore *writeConsoleLock;
static Semaphore *semGet;
static Semaphore *semPut;
static Semaphore *semGetString;
static Semaphore *semPutString;
static Semaphore *mainEnd;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    //readConsoleLock = new Semaphore("Console Read avail", 1);
    //writeConsoleLock = new Semaphore("Console Write Done", 1);
    
    semGet = new Semaphore("Read Char", 1);
    semPut = new Semaphore("Write Char", 1);
    semGetString = new Semaphore(" Read String", 1);
    semPutString = new Semaphore("Write String", 1); 
    mainEnd = new Semaphore("Wait at Main End for other threads to finish",1);
    console = new Console (readFile, writeFile, ReadAvail, WriteDone, 0);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeDone;
    delete readAvail;
    //delete readConsoleLock;
    //delete writeConsoleLock;
    delete semPut;
    delete semGetString;
    delete semPutString;
    delete semGet;
    delete mainEnd;

}


void SynchConsole::SynchPutChar(const char ch) {
    //
      
   // writeConsoleLock->P ();    //Lock 

    semPut->P();

     console->PutChar(ch);  //echo it
     writeDone->P ();

    semPut->V();
    //writeConsoleLock->V ();



}

int SynchConsole::SynchGetChar() {
//
    int ch;
    //Acquire the Lock
     semGet->P();
    readAvail->P ();  //Wait for character to arrive
    //ch = (int)console->GetChar();
    if(console->feof()) {
            
            semGet->V();
            return EOF;
    }
    else {
      ch = console->GetChar();
      //Release Lock
      semGet->V();
      return ch;
    }

    

 

//return ch;
}


//Read from Memory
void SynchConsole:: copyStringFromMachine( int from, char *to, unsigned size) {
  int tempValue;
  unsigned int i = 0;
  while(i<size) {
    machine->ReadMem(from + i , 1, &tempValue);  //reading 1 byte
    if(tempValue == '\0')  
      break;
    to[i] = (char )tempValue;  // Converting int to char as Memread takes int as an argument
    i++;
  }
  to[i] ='\0'; //For Security Purposes as defined in the Part V of Step 2
}

//Writes to memory
void SynchConsole::copyStringToMachine(char *from, int to, unsigned size) {

    unsigned  j, ch;

    for (j = 0; j < size - 1 && from[j] != '\0'; j++) {
       ch =  (int)from[j];
       machine->WriteMem(to + j, 1, ch);
    }

   machine->WriteMem(to + j, 1, 0);
}




void SynchConsole::SynchPutString(const char s[]) {
// ...
    int i = 0;
    //Acquire Lock to the Buffer
    //writeConsoleLock->P ();   //Lock 
    semPutString->P();

    while(s[i] != '\0' && i < MAX_STRING_SIZE-1) {
        SynchPutChar(s[i]);
        i++;

    }
    SynchPutChar('\0');

    semPutString->V();
  // writeConsoleLock->V ();  // Release lock after finishing writing
}

void SynchConsole::SynchGetString(char *s, int n) {
//
    int i;
  //char localBuffer[MAX_STRING_SIZE];  //Local buffer


   //readConsoleLock->P ();   //Lock 

      semGetString->P();

    for(i=0; i < n-1 ; i++) {
        s[i] = SynchGetChar();

        if (s[i] == EOF )
          break;
        if(s[i] == '\n' || s[i] == '\0')
            break;
    }
    s[i] = '\0';


   //readConsoleLock->V ();

      semGetString->V();

}





void SynchConsole::SynchPutInt(int n) {
  char *buf = new char[MAX_STRING_SIZE];
  snprintf(buf,MAX_STRING_SIZE,"%d",n);
  SynchPutString(buf);
  delete [] buf;
}




void SynchConsole::SynchGetInt(int *n) {
    char *buf = new char[MAX_STRING_SIZE];
    SynchGetString(buf,MAX_STRING_SIZE);
    sscanf(buf, "%d", n);
    delete [] buf;



}


// For main thread to wait for other threads to finish
void SynchConsole::threadCreateSem() {

  mainEnd->V(); // wait at main end
}

void SynchConsole::threadDestroySem() {

  mainEnd->P();
}

#endif  //CHANGED
