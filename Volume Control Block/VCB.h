/**************************************************************
* Class:  CSC-415-03 
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954)
* Student ID:
* Project: File System Project
*
* File: VCB.h
*
* Description: Header file for the functions required for our volume control block
*
**************************************************************/

//I just included all the same required libraries from other header files

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "fsLow.h"
#include "bitMap.h"
#include "mfs.h"

#define DATA_BLOCKS_PER_INODE	4 //# of datablocks per inode
#define VCB_START_BLOCK		0


#ifndef _MAKEVOL
#define _MAKEVOL


/* helpful resource: https://www3.nd.edu/~pbui/teaching/cse.30341.fa18/project06.html */

typedef struct { //structure to hold all the data of our VCB for ease of access
  	char header[16];
  	uint64_t volumeSize;
  	uint64_t blockSize;
  	uint64_t diskSizeBlocks;
  	uint64_t vcbStartBlock;
  	uint64_t totalVCBBlocks;
  	uint64_t inodeStartBlock;
  	uint64_t totalInodes;
 	uint64_t totalInodeBlocks;
  	uint64_t freeMapSize;
  	uint32_t freeMap[];
} VCB; //basic struct for what I think we're going to need that makes up the data for the VCB

#endif

//VCVB* openVCBpointer (i changed the var name in the other file because we had some disputes between the var among each of our owrk)

uint64_t intDiv(uint64_t, uint64_t); //in case we want to round up integer division

int allocateVCB(VCB**); 

uint64_t ReadDisk(void*, uint64_t, uint64_t); //read directly to the disk

uint64_t WriteDisk(void*, uint64_t, uint64_t); //write to the disk

void FreeVCB(void*, uint64_t, uint64_t); //free up the vcb

int checkIfStorageIsAvalibale(int numberOfRequestedBlocks); //check if theres open contiguous blocks return either 0 or 1 if whether or not theres space or not

uint64_t getFreeBlock(); //returns the first free block available

uint64_t readVCB(); //read VCB into memory

uint64_t writeVCB(); //write VCB into memory

VCB* getVCB();

void printVCB();

int createVCB(char*, uint64_t, uint64_t);
/* CreateVCB will create a new volume with the file name, volumesize, and blocksize
Probably needs to initialize the vcb and nodes */ 

void openVCB(char*);

void closeVCB();
