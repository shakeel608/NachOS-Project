// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"
#include "synch.h"

#define CompleteFileNameMaxLen 250
#define MaxOpenFilesInSystem   10

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
				// calls to UNIX, until the real file system
				// implementation is available
class FileSystem {
  public:
    FileSystem(bool format) {}

    bool Create(const char *name, int initialSize) { 
	int fileDescriptor = OpenForWrite(name);

	if (fileDescriptor == -1) return FALSE;
	Close(fileDescriptor); 
	return TRUE; 
	}

    OpenFile* Open(char *name) {
	  int fileDescriptor = OpenForReadWrite(name, FALSE);

	  if (fileDescriptor == -1) return NULL;
	  return new OpenFile(fileDescriptor);
      }

    bool Remove(char *name) { return Unlink(name) == 0; }

};

#else // FILESYS

class Semaphore;

class OpenFilesTableEntry {
  public:
    bool inUse;
    char fileName[CompleteFileNameMaxLen + 1];
    int pro; //the process that opened this file
};


class FileSystem {
  public:
    FileSystem(bool format);		// Initialize the file system.
					// Must be called *after* "synchDisk" 
					// has been initialized.
    					// If "format", there is nothing on
					// the disk, so initialize the directory
    					// and the bitmap of free blocks.

    bool Create(const char *name, int initialSize);  	
					// Create a file (UNIX creat)

    OpenFile* Open(const char *name); 	// Open a file (UNIX open)

    bool Remove(const char *name); 	// Delete a file (UNIX unlink)

    void List();			// List all the files in the file system

    void Print();			// List all the files and their contents

    //Add an entry to the openFilesTable
    //Returns the index of the entry or -1 if there is no space
    int AddToOpenFilesTable(const char *name);

    //  Look up file name in openFilesTable, and return the index in the table
    //  Return -1 if the name isn't  in the directory.
    //  "name" -- the file name to look up
    int FindOpenFileIndex(const char *name);


    void DeleteEntriesOfProcess(int pro);

    void WriteBackHeaders(FileHeader *hdr, FileHeader *hdrInd1, FileHeader *hdrInd2, FileHeader **hdrInd3, int size);


    //*********   SYSCALLS HANDLERS ***************************
    bool MakeDirectory(const char *name); //Create a directory
    bool ChangeDirectory(const char *name); //Change directory
    bool RemoveSyscall(const char *name); //Remove file or dir


    //Handle a open syscall
    //Open or creates a file
    //      name -- name of the file
    //      create --   0 to not create a file
    //                  1 to create a file if it does not exists
    //Return the file descriptor on succes or -1 if there is an error
    int OpenSyscall(const char *name, int create, int size);

    //Read count bytes from the file referred by fileDescriptor
    //      fileDescriptor -- the file descriptor of the file to be read 
    //                        this corresponds to the index in the openFilesTable
    //      count -- The intended number of bytes to be read
    //Return the number of bytes acutally read or -1 if there is an error
    int ReadSyscall(int fileDescriptor, int count);

    //Write in the file referred by fileDescriptor
    //      fileDescriptor -- the file descriptor of the file to be read 
    //                        this corresponds to the index in the openFilesTable
    //      buff -- A pointer to the data to be written
    //Return the number of bytes acutally written or -1 if there is an error
    int WriteSyscall(int fileDescriptor, const char *buf, int size);

    //closes  a  file descriptor, so that it no longer refers to any file and 
    //may be reused
    //Return 0 on success or -1 if there is an error
    //      fileDescriptor -- the file descriptor of the file to be closed
    int CloseSyscall(int fileDescriptor);

    int LseekSyscall(int fileDescriptor, int offset);

    //***********************************************************


  private:
   OpenFile* freeMapFile;		// Bit map of free disk blocks,
					// represented as a file
   OpenFile* directoryFile;		// "Root" directory -- list of 
					// file names, represented as a file

   OpenFilesTableEntry *openFilesTable;

   Semaphore *filesys_lock; //Semaphore to protect access in 
            //the file system
};

#endif // FILESYS

#endif // FS_H
