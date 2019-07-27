#include "syscall.h"

int main ()
{
	// open(0, 0, 0);
	// read(0, 0);
	// write(0, 0);
	// close(0);

	int fd1 = open("hola", 0, 0);
	int fd2 = open("holaM", 0, 0);
	read(fd1, 100);	
	read(fd2, 100);
}
