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
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    
    fileType = 'f';
    createTime = clock();
    lastModifyTime = clock();
    lastOpenTime = clock();


    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < (numSectors + NumDirect-1)/(NumDirect)*(NumDirect+1) )
	return FALSE;		// not enough space
    
    this->firstIndexSector = freeMap->Find();

    int buf[NumDirect+1];
    int indexSector = this->firstIndexSector;
    for (int i = 0; i < numSectors; i++) {
	   buf[i%NumDirect] = freeMap->Find();
       if (i%NumDirect == NumDirect-1 || i == numSectors-1) {
            if (i != numSectors-1) buf[NumDirect] = freeMap->Find();
            synchDisk->WriteSector(indexSector, (char *)buf);
            indexSector = buf[NumDirect];
       }
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
    int indexSector = this->firstIndexSector;
    int buf[NumDirect+1];
    synchDisk->ReadSector(indexSector, (char *)buf);
    freeMap->Clear(indexSector);

    for (int i = 0; i < numSectors; i++) {
    	int sector = buf[i%NumDirect];
        freeMap->Clear(sector);

        if (i%NumDirect == NumDirect-1 && i != numSectors-1) {
            indexSector = buf[NumDirect];
            synchDisk->ReadSector(indexSector, (char *)buf);
            freeMap->Clear(indexSector);
        } 
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
    int indexSector = this->firstIndexSector;
    int sector = offset/SectorSize;
    int buf[NumDirect+1];
    while (1>0) {
        synchDisk->ReadSector(indexSector, (char *)buf);
        if (sector < NumDirect) {
            return buf[sector];
        }
        sector -= NumDirect;
        indexSector = buf[NumDirect];
    }
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
    /*
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
  
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
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
*/
}

//return false if there's no extra sectors for addSize
bool 
FileHeader::addFileSize(BitMap *freeMap, int addSize) {
    int addSectors = divRoundUp(this->numBytes+addSize, SectorSize) - this->numSectors;
    if (addSectors == 0) {
        this->numBytes += addSize;
        return true;
    }


    if (freeMap->NumClear() < (addSectors + NumDirect-1)/(NumDirect)*(NumDirect+1) ) 
        return false;

    int indexSector = this->firstIndexSector;
    int sector = this->numSectors;
    int buf[NumDirect+1];
    
    while (sector >= NumDirect) {
        synchDisk->ReadSector(indexSector, (char *)buf);
        sector -= NumDirect;
        indexSector = buf[NumDirect];
    } 

    synchDisk->ReadSector(indexSector, (char *)buf);
    for (int i = 0; i < addSectors; i++) {
        //sector must be less than NumDirect, so there's space for buf
        buf[(sector+i)%NumDirect] = freeMap->Find();
        
        if (i == addSectors-1) {
            synchDisk->WriteSector(indexSector, (char *)buf);
        }
        
        if (i != addSectors-1 && (sector+i)%NumDirect == NumDirect-1) {
            buf[NumDirect] = freeMap->Find();
            synchDisk->WriteSector(indexSector, (char *)buf);
            indexSector = buf[NumDirect];        
        }
    }


    this->numBytes += addSize;
    this->numSectors = divRoundUp(this->numBytes+addSize, SectorSize) - this->numSectors;
}