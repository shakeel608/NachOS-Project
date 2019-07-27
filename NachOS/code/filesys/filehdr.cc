// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize, 
                    FileHeader **ind1, FileHeader **ind2, FileHeader ***ind3)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);

    int numIndirect1 = NumIndirect1;
    int numIndirect2 = NumIndirect2;
    int numDirect = NumDirect;

    if(numSectors <= numDirect){
        if (freeMap->NumClear() < numSectors)
        	return FALSE;		// not enough space

        for (int i = 0; i < numSectors; i++)
    	   dataSectors[i] = freeMap->Find();

        *ind1 = NULL;
        *ind2 = NULL;
        *ind3 = NULL;
    }
    else if(numSectors <= numIndirect1){
        if (freeMap->NumClear() < numSectors + 1)
            return FALSE;       // not enough space

        for (int i = 0; i < numDirect; i++)
            dataSectors[i] = freeMap->Find();

        *ind1 = new FileHeader;
        if(!(*ind1)->AllocateIndirection1(freeMap, numSectors - numDirect + 1)){
            delete *ind1;
            return FALSE;
        }
        *ind2 = NULL;
        *ind3 = NULL;
    }

    else if(numSectors <= numIndirect2){
        //Calculate the number of level 2 indirect blocks
        int sectorsRemaining;
        int numL2Blocks;

        sectorsRemaining = numSectors - numIndirect1 + 1;
        numL2Blocks = divRoundUp(sectorsRemaining, numDirect);
        
        if (freeMap->NumClear() < numSectors + 2 + numL2Blocks)
            return FALSE;       // not enough space

        for (int i = 0; i < numDirect; i++)
            dataSectors[i] = freeMap->Find();

        *ind1 = new FileHeader;
        if(!(*ind1)->AllocateIndirection1(freeMap, numDirect)){
            delete *ind1;
            return FALSE;
        }

        *ind2 = new FileHeader;
        *ind3 = new FileHeader*[numL2Blocks];
        if(!(*ind2)->AllocateIndirection2(freeMap, sectorsRemaining, ind3)){
            delete[] ind3;
            delete *ind2;
            return FALSE;
        }

    }

    else  {
        //File too big
        return FALSE;
    }

    return TRUE;
}


bool
FileHeader::AllocateIndirection1(BitMap *freeMap, int numSectorsRemaining) 
{ 
    int numDirect = NumDirect;

    if(numSectorsRemaining > numDirect)
        return FALSE; //It should fit

    if (freeMap->NumClear() < numSectorsRemaining)
        return FALSE;       // not enough space

    for (int i = 0; i < numSectorsRemaining; i++)
        dataSectors[i] = freeMap->Find();

    return TRUE;
}


bool
FileHeader::AllocateIndirection2(BitMap *freeMap, int numSectorsRemaining, FileHeader ***ind3) 
{ 
    int numDirect = NumDirect;
    int numBlocks = divRoundUp(numSectorsRemaining, numDirect);

    for (int i = 0; i < numBlocks; i++){

        dataSectors[i] = freeMap->Find();

        FileHeader *ind = new FileHeader;
        if(numSectorsRemaining > numDirect){
            if(!ind->AllocateIndirection1(freeMap, numDirect)){
                delete ind;
                return FALSE;
            }
        }
        else{
            if(!ind->AllocateIndirection1(freeMap, numSectorsRemaining)){
                delete ind;
                return FALSE;
            }
        }

        numSectorsRemaining -= numDirect;
        (*ind3)[i] = ind;
    }

    return TRUE;
}


//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
 
    int numIndirect1 = NumIndirect1;
    int numIndirect2 = NumIndirect2;
    int numDirect = NumDirect;

    if(numSectors <= numDirect){
        for (int i = 0; i < numSectors; i++) {
            ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }
    }

    else if(numSectors <= numIndirect1){

        int sector;

        FileHeader *hdr = new FileHeader;
        hdr->FetchFrom(dataSectors[numDirect-1]);
        
        for (int i = 0; i < numSectors; i++) {
            if(i < numDirect - 1){    
                ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
                freeMap->Clear((int) dataSectors[i]);
            }
            else{
                sector = hdr->GetSector(i - numDirect + 1); 
                ASSERT(freeMap->Test(sector));  // ought to be marked!
                freeMap->Clear(sector);
            }
        }
        ASSERT(freeMap->Test((int) dataSectors[numDirect - 1]));  // ought to be marked!
        freeMap->Clear((int) dataSectors[numDirect -1]);

        delete hdr;
    }

    else if(numSectors <= numIndirect2){

        int sector;
        int tblSector;
        int numBlock;

        FileHeader *hdr = new FileHeader;
        hdr->FetchFrom(dataSectors[numDirect-2]);
        FileHeader *hdrInd1 = new FileHeader;
        hdrInd1->FetchFrom(dataSectors[numDirect-1]);
        FileHeader *hdrTbl = new FileHeader;
        
        for (int i = 0; i < numSectors; i++) {
            if(i < numDirect - 2){    
                ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
                freeMap->Clear((int) dataSectors[i]);
            }
            else if(i < numIndirect1 - 1){
                sector = hdr->GetSector(i - numDirect + 2);
                ASSERT(freeMap->Test(sector));  // ought to be marked!
                freeMap->Clear(sector);
            }
            else{
                numBlock = (i - numIndirect1 + 1) / numDirect;
                tblSector = hdrInd1->GetSector(numBlock);
                hdrTbl->FetchFrom(tblSector);
                sector = hdrTbl->GetSector((i - numIndirect1 + 1)%numDirect);
                ASSERT(freeMap->Test(sector));  // ought to be marked!
                freeMap->Clear(sector);                
                if((i - numIndirect1 + 1)%numDirect == numDirect - 1){
                    ASSERT(freeMap->Test(tblSector));  // ought to be marked!
                    freeMap->Clear(tblSector);
                }
            }
        }
        freeMap->Clear(tblSector);
        ASSERT(freeMap->Test((int) dataSectors[numDirect - 2]));  // ought to be marked!
        freeMap->Clear((int) dataSectors[numDirect -2]);
        ASSERT(freeMap->Test((int) dataSectors[numDirect - 1]));  // ought to be marked!
        freeMap->Clear((int) dataSectors[numDirect -1]);

        delete hdr;
        delete hdrInd1;
        delete hdrTbl;
    }

}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    int numIndirect1 = NumIndirect1;
    int numIndirect2 = NumIndirect2;
    int numDirect = NumDirect;

    if(numSectors <= numDirect)
        return(dataSectors[offset / SectorSize]);

    if(numSectors <= numIndirect1){
        int index; 
        int sector;

        index = offset / SectorSize;
        
        if(index < numDirect - 1)
            return(dataSectors[index]);

        FileHeader *hdr = new FileHeader;
        hdr->FetchFrom(dataSectors[numDirect-1]);
        sector = hdr->GetSector(index - numDirect + 1); 
        delete hdr;

        return sector;
    }

    if(numSectors <= numIndirect2){
        int index;
        int tblSector;
        int sector;
        int numBlock;

        index = offset / SectorSize;
        
        if(index < numDirect - 2)
            return(dataSectors[index]);

        if(index < numIndirect1 - 1){
            FileHeader *hdr = new FileHeader;
            hdr->FetchFrom(dataSectors[numDirect-2]);
            sector = hdr->GetSector(index - numDirect + 2);
            delete hdr;
            return sector;
        }

        numBlock = (index - numIndirect1 + 1) / numDirect;

        FileHeader *hdrInd1 = new FileHeader;
        hdrInd1->FetchFrom(dataSectors[numDirect-1]);
        tblSector = hdrInd1->GetSector(numBlock);
        delete hdrInd1;
        
        FileHeader *hdrTbl = new FileHeader;
        hdrTbl->FetchFrom(tblSector);
        sector = hdrTbl->GetSector((index - numIndirect1 + 1)%numDirect);
        delete hdrTbl;

        return sector;

    }

    ASSERT(FALSE);
    return 0;
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];
    int tmpNumSect;

    int numDirect = NumDirect;

    tmpNumSect = numSectors;
    if(numSectors > numDirect){
        printf("WARNNING printing only direct references\n");
        tmpNumSect = numDirect;
    }

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < tmpNumSect; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < tmpNumSect; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}

int 
FileHeader::GetSector(int index){
    return dataSectors[index];
}
