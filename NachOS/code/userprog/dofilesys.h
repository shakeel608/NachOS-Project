#include "copyright.h"
#include "system.h"

extern void do_cpunix(int arg1, int arg2);
extern void do_filepr(int nm);
extern void do_DIR();
extern int do_rm(int nm);
extern void do_ls();
extern int do_mkdir(int nm);
extern int do_cd(int nm);
extern int do_open(int nm, int create, int size);
extern int do_read(int fileDescriptor, int count);
extern int do_write(int fileDescriptor, int buf, int size);
extern int do_close(int fileDescriptor);
extern int do_lseek(int fileDescriptor, int offset);
