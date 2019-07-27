#include "syscall.h"

 void thread1(void* a)
 {
 	int fd = open("hola", 0, 0);
 	read(fd, 100);
	close(fd);
 	rm("hola");

 	UserThreadExit();
 }


int main ()
{
	// open(0, 0, 0);
	// read(0, 0);
	// write(0, 0);
	// close(0);
	int t1 = UserThreadCreate(thread1, 0);
	int fd = open("hola", 0, 0);
 	read(fd, 100);
 	UserThreadJoin(t1);
	close(fd);
	rm("hola");

// 	remove("hola");
// 	int fd1 = open("holaJ", 0, 0);
// 	write(fd1, "hello", 5);
//	read(fd, 100);
// 	read(fd1, 100);
//	close(fd);
//	close(fd1);

//	ForkExec("test2");
}
