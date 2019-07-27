#include "copyright.h"
#include "system.h"

typedef struct threadArgs
{
	int f;
	int arg;	
	int sp;
	//int exit; //used to define the default return address
}threadArgs_t;

extern void do_Exit();
extern void do_UserThreadJoin(int arg);
extern int do_UserThreadCreate(int f, int arg);
extern void do_UserThreadExit();
extern void do_UserSemInit(int semID, int semCounter);
extern void do_UserSemP(int semID);
extern void do_UserSemV(int semID);
