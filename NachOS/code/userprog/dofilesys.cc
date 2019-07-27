#include "copyright.h"
#include "dofilesys.h"

extern void Copy (const char *unixFile, const char *nachosFile);
extern void Print (char *file);

#ifndef FILESYS_STUB
static void getStringFromMachine(int memAddr, char* string){

	int val;
	for(int i = 0; i < MAX_STRING_SIZE; i++){
		machine->ReadMem(memAddr+i, 1, &val);
		string[i] = val;
		if(val == '\0')
			return;
	}
}
#endif // NOT FILESYS_STUB


void do_cpunix(int from, int to){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'cpunix' in FILESYS_STUB flavor\n");
	#else // FILESYS

	char nameFrom[MAX_STRING_SIZE];
	char nameTo[MAX_STRING_SIZE];
	getStringFromMachine(from, nameFrom);
	getStringFromMachine(to, nameTo);

	if(nameFrom[0] == '\0' || nameTo[0] == '\0'){
		printf("Two file names should be provided\n");
		return;
	}

	Copy (nameFrom, nameTo);

	#endif // FILESYS
}

void do_filepr(int nm){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'filepr' in FILESYS_STUB flavor\n");
	#else // FILESYS

	char name[MAX_STRING_SIZE];
	getStringFromMachine(nm, name);

	if(name[0] == '\0'){
		printf("A file name should be provided\n");
		return;
	}

	Print(name);

	#endif // FILESYS
}

void do_DIR(){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'DIR' in FILESYS_STUB flavor\n");
	#else // FILESYS

	fileSystem->Print ();

	#endif // FILESYS
}



//Return 0 if the file is deleted, -1 if there was an error
//"nm" -- pointer to the text name of the file
int do_rm(int nm){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'rm' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	char name[MAX_STRING_SIZE];
	getStringFromMachine(nm, name);

	if(name[0] == '\0'){
		printf("A file name should be provided\n");
		return -1;
	}

	if(fileSystem->RemoveSyscall(name))
		return 0;

	printf("Error removing '%s'\n", name);	

    return -1;

	#endif // FILESYS
}

// List the files
void do_ls(){

	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'ls' in FILESYS_STUB flavor\n");

	#else // FILESYS
	fileSystem->List ();

	#endif // FILESYS
}

//Return 0 if everything is ok, otherwise return -1 
//"nm" -- pointer to the text name of the directory
int do_mkdir(int nm){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'mkdir' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	char name[MAX_STRING_SIZE];
	getStringFromMachine(nm, name);

	if(name[0] == '\0'){
		printf("A file name should be provided\n");
		return -1;
	}

	if(fileSystem->MakeDirectory(name))
		return 0;

	printf("Couldn't create '%s' directory\n", name);	

    return -1;

	#endif // FILESYS

}

//Return 0 if everything is ok, otherwise return -1 
//"nm" -- pointer to the text name of the directory
int do_cd(int nm){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	printf("It is not possible to execute 'cd' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	char name[MAX_STRING_SIZE];
	getStringFromMachine(nm, name);

	if(name[0] == '\0'){
		printf("A directory name should be provided\n");
		return -1;
	}

	if(fileSystem->ChangeDirectory(name))
		return 0;

	printf("Couldn't open '%s' directory\n", name);	

    return -1;

	#endif // FILESYS

}

//Open or creates a file
//		"nm" -- pointer to the text name of the file
//		create -- 	0 to not create a file
//					1 to create a file if it does not exists
//		size -- Size of the file in case it is created
//Return the file descriptor on succes or -1 if there is an error
int do_open(int nm, int create, int size){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	DEBUG('s', "It is not possible to execute 'open' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	char name[MAX_STRING_SIZE];
	int fileDescriptor;

	getStringFromMachine(nm, name);
	if(name[0] == '\0'){
		DEBUG('s', "A file name should be provided\n");
		return -1;
	}

	fileDescriptor = fileSystem->OpenSyscall(name, create, size);
	if(fileDescriptor == -1){
		DEBUG('s', "Could not open '%s' file\n", name);
		return -1;
	}

	return fileDescriptor;

	#endif // FILESYS

}

//Read count bytes from the file referred by fileDescriptor
//		fileDescriptor -- the file descriptor of the file to be read
//		count -- The intended number of bytes to be read
//Return the number of bytes acutally read or -1 if there is an error
//Side effect: The seek position in the file is incremented
int do_read(int fileDescriptor, int count){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	DEBUG('s', "It is not possible to execute 'read' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	int returnVal;

	returnVal = fileSystem->ReadSyscall(fileDescriptor, count);
	if(returnVal == -1){
		DEBUG('s', "Could not read the file\n");
		return -1;
	}

	return returnVal;

	#endif // FILESYS

}

//Write the contents of buf in the file referred by fileDescriptor
//		fileDescriptor -- the file descriptor of the file to be written
//		buf -- A pointer to the data to be written
//		size -- the number of bytes to be written
//Return the number of bytes acutally written or -1 if there is an error
//Side effect: The seek position in the file is incremented
int do_write(int fileDescriptor, int buf, int size){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	DEBUG('s', "It is not possible to execute 'write' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	int returnVal;
	char bufToWrite[MAX_STRING_SIZE];
	
	getStringFromMachine(buf, bufToWrite);

	returnVal = fileSystem->WriteSyscall(fileDescriptor, bufToWrite, size);
	if(returnVal == -1){
		DEBUG('s', "Could not write the file\n");
		return -1;
	}

	return returnVal;

	#endif // FILESYS

}

//closes  a  file descriptor, so that it no longer refers to any file and 
//may be reused
//Return 0 on success or -1 if there is an error
//		fileDescriptor -- the file descriptor of the file to be closed
int do_close(int fileDescriptor){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	DEBUG('s', "It is not possible to execute 'close' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	return fileSystem->CloseSyscall(fileDescriptor);

	#endif // FILESYS

}


//moves the offset of a open file
//		fileDescriptor -- the file descriptor of the file
//		offset -- the new offset of the file
//Returns 0 if everything is ok, -1 if there was an error
int do_lseek(int fileDescriptor, int offset){
	
	#ifdef FILESYS_STUB //In case we are in FILESYS_STUB flavor
	DEBUG('s', "It is not possible to execute 'lseek' in FILESYS_STUB flavor\n");
	return -1;
	#else // FILESYS

	return fileSystem->LseekSyscall(fileDescriptor, offset);

	#endif // FILESYS

}

