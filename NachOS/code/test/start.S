/* Start.s
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#include "syscall.h"

        .text
        .align  2

/* -------------------------------------------------------------
 * __start
 *	Initialize running a C program, by calling "main".
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

	.globl __start
	.ent	__start
__start:
	jal	main
	move	$4,$0
	jal	Exit	 /* if we return from main, exit(0) */
	.end __start

/* -------------------------------------------------------------
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r2, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r4, arg2 is
 *	in r5, arg3 is in r6, arg4 is in r7)
 *
 * 	The return value is in r2. This follows the standard C calling
 * 	convention on the MIPS.
 * -------------------------------------------------------------
 */

	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,SC_Halt
	syscall
	j	$31
	.end Halt

	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,SC_Exit
	syscall
	j	$31
	.end Exit

	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,SC_Exec
	syscall
	j	$31
	.end Exec

	.globl Join
	.ent	Join
Join:
	addiu $2,$0,SC_Join
	syscall
	j	$31
	.end Join

	.globl Create
	.ent	Create
Create:
	addiu $2,$0,SC_Create
	syscall
	j	$31
	.end Create

	.globl Open
	.ent	Open
Open:
	addiu $2,$0,SC_Open
	syscall
	j	$31
	.end Open

	.globl Read
	.ent	Read
Read:
	addiu $2,$0,SC_Read
	syscall
	j	$31
	.end Read

	.globl Write
	.ent	Write
Write:
	addiu $2,$0,SC_Write
	syscall
	j	$31
	.end Write

	.globl Close
	.ent	Close
Close:
	addiu $2,$0,SC_Close
	syscall
	j	$31
	.end Close

	.globl Fork
	.ent	Fork
Fork:
	addiu $2,$0,SC_Fork
	syscall
	j	$31
	.end Fork

	.globl Yield
	.ent	Yield
Yield:
	addiu $2,$0,SC_Yield
	syscall
	j	$31
	.end Yield

/* Syscall for PutChar */

	.globl PutChar
	.ent	PutChar
PutChar:
	addiu $2,$0,SC_PutChar
	syscall
	j	$31
	.end PutChar

/*Syscall for PutString */

  .globl PutString
  .ent	PutString
PutString:
  addiu $2,$0,SC_PutString
  syscall
  j	$31
  .end PutString

/*Syscall for GetChar */

	.globl GetChar
	.ent	GetChar
GetChar:
	addiu $2,$0,SC_GetChar
	syscall
	j	$31
	.end GetChar


/*Syscall for GetString */

	.globl GetString
	.ent	GetString
GetString:
	addiu $2,$0,SC_GetString
	syscall
	j	$31
	.end GetString

/*Syscall for GetInt */

	.globl GetInt
	.ent	GetInt
GetInt:
	addiu $2,$0,SC_GetInt
	syscall
	j	$31
	.end GetInt


/*Syscall for SC_PutInt */

	.globl PutInt
	.ent	GetInt
PutInt:
	addiu $2,$0,SC_PutInt
	syscall
	j	$31
	.end PutInt


/*Syscall for UserThreadCreate */

	.globl UserThreadCreate
	.ent	UserThreadCreate
UserThreadCreate:
	addiu $2,$0,SC_UserThreadCreate
	syscall
	j	$31
	.end UserThreadCreate

/*Syscall for UserTheadExit */

	.globl UserThreadExit
	.ent	UserThreadExit
UserThreadExit:
	addiu $2,$0,SC_UserThreadExit
	syscall
	j	$31
	.end UserThreadExit
	

/*Syscall for UserThreadJoin */


	.globl UserThreadJoin
	.ent	UserThreadJoin
UserThreadJoin:
	addiu $2,$0,SC_UserThreadJoin
	syscall
	j 	 $31
	.end UserThreadJoin


/*Syscall for ForkExec */

	.globl ForkExec
	.ent	ForkExec
ForkExec:
	addiu $2,$0,SC_ForkExec
	syscall
	j	$31
	.end ForkExec

	.globl UserWaitPid
	.ent	UserWaitPid
UserWaitPid:
	addiu $2,$0,SC_UserWaitPid
	syscall
	j	$31
	.end UserWaitPid


/*FileSys Syscalls */

	.globl cpunix
	.ent	cpunix
cpunix:
	addiu $2,$0,SC_cpunix
	syscall
	j	$31
	.end cpunix

	.globl filepr
	.ent	filepr
filepr:
	addiu $2,$0,SC_filepr
	syscall
	j	$31
	.end filepr

	.globl DIR
	.ent	DIR
DIR:
	addiu $2,$0,SC_DIR
	syscall
	j	$31
	.end DIR

	.globl rm
	.ent	rm
rm:
	addiu $2,$0,SC_rm
	syscall
	j	$31
	.end rm

	.globl ls
	.ent	ls
ls:
	addiu $2,$0,SC_ls
	syscall
	j	$31
	.end ls

	.globl mkdir
	.ent	mkdir
mkdir:
	addiu $2,$0,SC_mkdir
	syscall
	j	$31
	.end mkdir

	.globl cd
	.ent	cd
cd:
	addiu $2,$0,SC_cd
	syscall
	j	$31
	.end cd

	.globl open
	.ent	open
open:
	addiu $2,$0,SC_open
	syscall
	j	$31
	.end open

	.globl read
	.ent	read
read:
	addiu $2,$0,SC_read
	syscall
	j	$31
	.end read

	.globl write
	.ent	write
write:
	addiu $2,$0,SC_write
	syscall
	j	$31
	.end write

	.globl close
	.ent	close
close:
	addiu $2,$0,SC_close
	syscall
	j	$31
	.end close

	.globl lseek
	.ent	lseek
lseek:
	addiu $2,$0,SC_lseek
	syscall
	j	$31
	.end lseek

	.globl SemInit
	.ent	SemInit
SemInit:
	addiu $2,$0,SC_SemInit
	syscall
	j	$31
	.end SemInit

	.globl SemP
	.ent	SemP
SemP:
	addiu $2,$0,SC_SemP
	syscall
	j	$31
	.end SemP
	
	.globl SemV
	.ent	SemV
SemV:
	addiu $2,$0,SC_SemV
	syscall
	j	$31
	.end SemV


/* dummy function to keep gcc happy */
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main
