// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"

#ifndef FILESYS_STUB
#include "system.h"
#endif // NOT FILESYS_STUB

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)

static int currentSector;

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new BitMap(NumSectors);
        Directory *directory = new Directory(NumDirEntries);
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

        DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

    FileHeader *hdrInd1Map = NULL;
    FileHeader *hdrInd2Map = NULL;
    FileHeader **hdrInd3Map = NULL;
    FileHeader *hdrInd1Dir = NULL;
    FileHeader *hdrInd2Dir = NULL;
    FileHeader **hdrInd3Dir = NULL;

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize, &hdrInd1Map, &hdrInd2Map, &hdrInd3Map));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize, &hdrInd1Dir, &hdrInd2Dir, &hdrInd3Dir));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);
    WriteBackHeaders(mapHdr, hdrInd1Map, hdrInd2Map, hdrInd3Map, FreeMapFileSize);
    WriteBackHeaders(dirHdr, hdrInd1Dir, hdrInd2Dir, hdrInd3Dir, DirectoryFileSize);

    
    //Adding . and .. to the root
    directory->AddSubdirectory(".", DirectorySector);
    directory->AddSubdirectory("..", DirectorySector);


    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
     
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	directory->WriteBack(directoryFile);

	if (DebugIsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();

        delete freeMap; 
	delete directory; 
	delete mapHdr; 
	delete dirHdr;
    delete hdrInd1Map;
    delete hdrInd2Map;
    delete[] hdrInd3Map;
    delete hdrInd1Dir;
    delete hdrInd2Dir;
    delete[] hdrInd3Dir;
	}
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
    currentSector = DirectorySector;

    //Init Open Files Table
    openFilesTable = new OpenFilesTableEntry[MaxOpenFilesInSystem];
    for (int i = 0; i < MaxOpenFilesInSystem; i++){
        openFilesTable[i].inUse = FALSE;
        openFilesTable[i].fileName[0] = '\0';
        openFilesTable[i].pro = -1;
    }
    filesys_lock = new Semaphore("filesys lock", 1);
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(const char *name, int initialSize)
{
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG('f', "Creating file %s, size %d\n", name, initialSize);

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
        freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();	// find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
        else if (!directory->Add(name, sector))
            success = FALSE;	// no space in directory
        else {
            FileHeader *hdrInd1 = NULL;
            FileHeader *hdrInd2 = NULL;
            FileHeader **hdrInd3 = NULL;
    	    hdr = new FileHeader;
    	    if (!hdr->Allocate(freeMap, initialSize, &hdrInd1, &hdrInd2, &hdrInd3))
                success = FALSE;	// no space on disk for data
    	    else {	
    	    	success = TRUE;
                // everthing worked, flush all changes back to disk
        	    hdr->WriteBack(sector); 		
        	    directory->WriteBack(directoryFile);
        	    freeMap->WriteBack(freeMapFile);
                WriteBackHeaders(hdr, hdrInd1, hdrInd2, hdrInd3, initialSize);
    	    }
            delete hdrInd1;
            delete hdrInd2;
            delete[] hdrInd3;
            delete hdr;
        }
        delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(const char *name)
{ 
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG('f', "Opening file %s\n", name);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
    if (sector >= 0) 		
	openFile = new OpenFile(sector);	// name was found in directory 
    delete directory;
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(const char *name)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    OpenFile *subDirectoryFile;
    Directory *subDirectory;
    int sector;
    
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }

    //Check if trying to remove a non emtpy directory
    if(directory->IsDirectory(name)){
        subDirectoryFile = Open(name);
        if(subDirectoryFile){
            subDirectory = new Directory(NumDirEntries);
            subDirectory->FetchFrom(subDirectoryFile);
            if(!subDirectory->IsEmpty()){
                printf("Trying to remove a non-empty directory\n");
                delete directory;
                delete subDirectory;
                return FALSE;
            }
            delete subDirectory;
        }
    }

    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(directoryFile);        // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
} 


//Delete all entries of the Sytem OpenFilesTable that belong to 
//proces pro
void 
FileSystem::DeleteEntriesOfProcess(int pro){
    for (int i = 0; i < MaxOpenFilesInSystem; i++){
        if (openFilesTable[i].pro == pro && openFilesTable[i].inUse)
            openFilesTable[i].inUse = FALSE;
    }
}


//Add an entry to the openFilesTable
//Returns the index of the entry or -1 if there is no space
int
FileSystem::AddToOpenFilesTable(const char *name)
{ 
    for (int i = 0; i < MaxOpenFilesInSystem; i++){
        if (!openFilesTable[i].inUse) {
            openFilesTable[i].inUse = TRUE;
            strncpy(openFilesTable[i].fileName, name, CompleteFileNameMaxLen);
            openFilesTable[i].pro = currentThread->space->pro;
            return i;
        }
    }
    return -1;   // no space.
}

//  Look up file name in openFilesTable, and return the index in the table
//  Return -1 if the name isn't  in the directory.
//  "name" -- the file name to look up
int
FileSystem::FindOpenFileIndex(const char *name)
{
    for (int i = 0; i < MaxOpenFilesInSystem; i++)
        if (openFilesTable[i].inUse && !strncmp(openFilesTable[i].fileName, name, CompleteFileNameMaxLen))
        return i;
    return -1;      // name not in table
}

//Create a Directory -mkdir
bool
FileSystem::MakeDirectory(const char *name)
{
    Directory *directory;
    Directory *subDirectory;
    BitMap *freeMap;
    FileHeader *hdr;
    OpenFile *subDirectoryFile;
    int sector;
    bool success;

    DEBUG('f', "Creating directory %s\n", name);

    filesys_lock->P();
    
    directory = new Directory(NumDirEntries);

    //read the current directory table
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
        success = FALSE;          // directory already exists in parent directory
    else {  
        freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();   // find a sector to hold the directory header
        if (sector == -1)       
            success = FALSE;        // no free block for directory header 
        else if (!directory->AddSubdirectory(name, sector))
            success = FALSE;    // no space in parent directory
        else {
            FileHeader *hdrInd1 = NULL;
            FileHeader *hdrInd2 = NULL;
            FileHeader **hdrInd3 = NULL;
            hdr = new FileHeader;
            if (!hdr->Allocate(freeMap, DirectoryFileSize, &hdrInd1, &hdrInd2, &hdrInd3))
                success = FALSE;    // no space on disk for directory table
            else {  
                success = TRUE;

                //***Write changes bakc to disk ***//
                //Write the header of the new directory
                hdr->WriteBack(sector);
                //Writeback the updated directory table
                directory->WriteBack(directoryFile);
                //Writeback the updated free map
                freeMap->WriteBack(freeMapFile);

                //****Write the table of the new directory ***///
                //Create an empty Directory
                subDirectory = new Directory(NumDirEntries);
                subDirectory->AddSubdirectory(".", sector);
                subDirectory->AddSubdirectory("..", currentSector);
                //open the new directory table
                subDirectoryFile = new OpenFile(sector);
                //Write the directory table in the disk
                subDirectory->WriteBack(subDirectoryFile);

                WriteBackHeaders(hdr, hdrInd1, hdrInd2, hdrInd3, DirectoryFileSize);

                delete subDirectoryFile;
                delete subDirectory;
            }
            delete hdrInd1;
            delete hdrInd2;
            delete[] hdrInd3;
            delete hdr;
        }
        delete freeMap;
    }
    delete directory;
    filesys_lock->V();
    return success;
}

//Open Directory -cd
bool
FileSystem::ChangeDirectory(const char *name)
{
    OpenFile *subDirectoryFile;
    Directory *directory;

    filesys_lock->P();
    
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    if(!directory->IsDirectory(name)){
        delete directory;
        filesys_lock->V();
        return FALSE;
    }

    subDirectoryFile = Open(name);
    if(subDirectoryFile){
        currentSector = directory->Find(name);
        delete directoryFile;
        directoryFile = subDirectoryFile;
        delete directory;
        filesys_lock->V();
        return TRUE;
    }
    delete directory;
    filesys_lock->V();
    return FALSE;
}

//Handle a remove syscall
//Removes a file or a directory only if it is empty
//If is not empty, resturns FALSE
//      name -- name of the file
//Return the TRUE on success or FALSE on error
bool
FileSystem::RemoveSyscall(const char *name){

    //. and .. cannot be removed
    if(strncmp(name, ".", FileNameMaxLen) == 0)
        return FALSE;
    if(strncmp(name, "..", FileNameMaxLen) == 0)
        return FALSE;

    if(FindOpenFileIndex(name) != -1)
        return FALSE;   //the file is open by other proccess or thread

    return Remove(name);

}

//Handle a open syscall
//Open or creates a file
//      name -- name of the file
//      create --   0 to not create a file
//                  1 to create a file if it does not exists
//      size --initial size of the file in case the file is created
//Return the file descriptor on succes or -1 if there is an error
int
FileSystem::OpenSyscall(const char *name, int create, int size){

    OpenFile *openFile;
    int fileDescriptor;
    bool createResult;

    filesys_lock->P();

    if(FindOpenFileIndex(name) != -1){
        //File already in System OpenFilesTable, check if the current process
        //is the owner
        if(currentThread->space->FindOpenFileIndex(name) == -1){
            //File is not open in this process
            filesys_lock->V();
            return -1;  //File is already open by another process
        }
    }

    openFile = Open(name);
    if(openFile == NULL){ 
        
        //File was not found
        if(create == 0){
            filesys_lock->V();
            return -1;  //File does not exist and user didn't ask to create one 
        }
        
        //Create a new file
        createResult = Create(name, size);
        if(!createResult){
            filesys_lock->V();
            return -1; //Error creating the file
        }

        //Open recently created file
        openFile = Open(name);
    }

    if(FindOpenFileIndex(name) == -1){
        //The file doesn't have an entry in the sistem openFilesTable
        if(AddToOpenFilesTable(name) == -1){
            delete openFile;
            filesys_lock->V();
            return -1;  //Not enough space in the System openFilesTable        
        }
    }

    fileDescriptor = currentThread->space->AddToOpenFilesTable(name, openFile);
    if(fileDescriptor == -1){
        delete openFile;
        filesys_lock->V();
        return -1;  //Not enough space in the Process openFilesTable
    }

    filesys_lock->V();
    return fileDescriptor;
}


//Read count bytes from the file referred by fileDescriptor
//      fileDescriptor -- the file descriptor of the file to be read 
//                        this corresponds to the index in the openFilesTable
//      count -- The intended number of bytes to be read
//Return the number of bytes acutally read or -1 if there is an error
int
FileSystem::ReadSyscall(int fileDescriptor, int count){

    OpenFile *openFile;
    int i, amountRead;
    char *buffer;

    //Check if the file is open
    if(!currentThread->space->openFilesTable[fileDescriptor].inUse)
        return -1; //the file is not open anymore

    openFile = currentThread->space->openFilesTable[fileDescriptor].openFile;
    
    buffer = new char[count];
    amountRead = openFile->Read(buffer, count);
    for (i = 0; i < amountRead; i++)
        printf("%c", buffer[i]);
    
    delete [] buffer;

    return amountRead;
}

//Write in the file referred by fileDescriptor
//      fileDescriptor -- the file descriptor of the file to be read 
//                        this corresponds to the index in the openFilesTable
//      buff -- A pointer to the data to be written
//      size -- the number of bytes to be written
//Return the number of bytes acutally written or -1 if there is an error
int
FileSystem::WriteSyscall(int fileDescriptor, const char *buf, int size){

    OpenFile *openFile;

    //Check if the file is open
    if(!currentThread->space->openFilesTable[fileDescriptor].inUse)
        return -1; //the file is not open anymore

    openFile = currentThread->space->openFilesTable[fileDescriptor].openFile;
    
    // Copy the data
    return openFile->Write(buf, size);    

}


//closes  a  file descriptor, so that it no longer refers to any file and 
//may be reused
//Return 0 on success or -1 if there is an error
//      fileDescriptor -- the file descriptor of the file to be closed
int
FileSystem::CloseSyscall(int fileDescriptor){

    char *name;
    int systemTableEntryIndex;

    filesys_lock->P();

    //Check if the file is open
    if(!currentThread->space->openFilesTable[fileDescriptor].inUse){
        filesys_lock->V();
        return -1; //the file is not open anymore
    }

    //Remove Entry
    currentThread->space->openFilesTable[fileDescriptor].inUse = FALSE;

    //Check if there is the another reference to this file in this process
    name = currentThread->space->openFilesTable[fileDescriptor].fileName;
    if(currentThread->space->FindOpenFileIndex(name) == -1){
        //There are no more references, delete the entry in the System Table
        systemTableEntryIndex = FindOpenFileIndex(name);
        if(systemTableEntryIndex == -1)
            ASSERT (FALSE); //the entry must exist in the system openFilesTable
        openFilesTable[systemTableEntryIndex].inUse = FALSE;
    }
    
    filesys_lock->V();

    return 0;    

}

//Return 0 if everything is ok, -1 in case of error
int 
FileSystem::LseekSyscall(int fileDescriptor, int offset){

    OpenFile *openFile;

    //Check if the file is open
    if(!currentThread->space->openFilesTable[fileDescriptor].inUse)
        return -1; //the file is not open anymore

    openFile = currentThread->space->openFilesTable[fileDescriptor].openFile;
    
    openFile->Seek(offset);
    
    return 0;

}



void
FileSystem::WriteBackHeaders(FileHeader *hdr, 
                            FileHeader *hdrInd1, 
                            FileHeader *hdrInd2, 
                            FileHeader **hdrInd3,
                            int size){

    int sector;

    if(hdrInd1 != NULL && hdrInd2 == NULL){
        sector = hdr->GetSector(NumDirect - 1);
        hdrInd1->WriteBack(sector);
    }
    else if(hdrInd1 != NULL && hdrInd2 != NULL && hdrInd3 != NULL){
        
        int nSectors  = divRoundUp(size, SectorSize);
        int sectorsRemaining = nSectors - NumIndirect1 + 1;
        int numBlocks = divRoundUp(sectorsRemaining, NumDirect);
        
        sector = hdr->GetSector(NumDirect - 2);
        hdrInd1->WriteBack(sector);
        sector = hdr->GetSector(NumDirect - 1);
        hdrInd2->WriteBack(sector);
        for(int i = 0; i < numBlocks; i++){
            sector = hdrInd2->GetSector(i);
            hdrInd3[i]->WriteBack(sector);
        }
    }

}