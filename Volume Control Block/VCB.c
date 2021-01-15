/**************************************************************
* Class:  CSC-415-03 Fall 2020 
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954)
* Student ID:
* Project: File System Project
*
* File: VCB.c
*
* Description: This file is used to create the volume for our file system. It will hold our free space and the inodes on our disk
*
**************************************************************/
#define FILESIZE 520
#define maxSize 1240
#include "VCB.h"

int AreWeInitialized = 0;  // variable to make sure the VCB will start initialized

//init our diff data info (it's a bit diff than our struct so i had to reinitialize)
char header[55] = "***FileSystemProject***";
uint64_t volumeSize;
uint64_t blockSize;
uint64_t diskSizeBlocks;
uint32_t vcbStartBlock;
uint32_t totalVCBBlocks;
uint32_t inodeStartBlock;
uint32_t totalInodes;
uint32_t totalInodeBlocks;
uint32_t freeMapSize;

VCB* VCBpointer; //pointer of our struct to the memory of the VCB


uint64_t intDiv(uint64_t a, uint64_t b) {
  	return (a + b - 1) / b;
}

int allocateVCB(VCB** vcb_p) {
  	*vcb_p = calloc(totalVCBBlocks, blockSize); //allocate totalblocks and blocksize to pointer for easier use
  	return totalVCBBlocks;
}

uint64_t ReadDisk(void* buf, uint64_t blockCount, uint64_t blockPosition) {
  	if(!AreWeInitialized) { //Checks initialization
    		printf("ReadDisk: System not yet initialized!\n");
    		return 0;
  	}
  	if(blockPosition + blockCount > VCBpointer->diskSizeBlocks) { //logic to make sure correct amount is read and not more on accident
    		printf("ReadDisk: Error reading requested data. Invalid blocks.\n");
    		return 0;
  	}
  	LBAread(buf, blockCount, blockPosition); //read to disk
  	return blockCount;
}

uint64_t WriteDisk(void* buf, uint64_t blockCount, uint64_t blockPosition) {
  	if(!AreWeInitialized) {
    		printf("WriteDisk: System not initialized!\n");
    		return 0;
  	}
  	if(blockPosition + blockCount > VCBpointer->diskSizeBlocks) { //same logic for checking higher than block amount
    		printf("WriteDisk: Invalid block input.\n");
    		return 0;
  	}
  	LBAwrite(buf, blockCount, blockPosition);
  	for(int i=0; i<blockCount; i++) {
    		setBitMap(VCBpointer -> freeMap, blockPosition + i); //setting our free space bit vector into the freemap from our struct
  	}
  	writeVCB();
  	return blockCount;
}

void FreeVCB(void* buf, uint64_t blockCount, uint64_t blockPosition) {
  	if(!AreWeInitialized) {
    		printf("FreeVCB: System not initialized!\n");
    		return;
  	}
  	if(blockPosition + blockCount > VCBpointer -> diskSizeBlocks) { //consistent logic as all other functions
    		printf("FreeVCB: Invalid block range.\n");
    		return;
  	}
  	for(int i=0; i<blockCount; i++) {
    	clearBitMap(VCBpointer -> freeMap, blockPosition + i); //freeing our free space bit vec
  	}
  	writeVCB();
}

uint64_t getFreeBlock(){ //returns the first free block available
  	for (int index = 0; index < diskSizeBlocks; index++){
    		if(findBitMap(VCBpointer -> freeMap, index) == 0) {
			return index; 
    		}
  	}
 	return -1;
}

uint64_t readVCB() { //read vcb into memory
  	if(!AreWeInitialized) {
    		printf("readVCB: System no initialized!\n");
    		return 0;
 	 }

  	uint64_t blocksRead = LBAread(VCBpointer, totalVCBBlocks, VCB_START_BLOCK);
  	printf("Read VCB in %d blocks starting at block %d.\n", totalVCBBlocks, VCB_START_BLOCK);
  	return blocksRead;
}

uint64_t writeVCB() { //write vcb into memory
  	if(!AreWeInitialized) {
   		 printf("writeVCB: System not initialized.\n");
    		return 0;
  	}

  	uint64_t blocksWritten = LBAwrite(VCBpointer, totalVCBBlocks, VCB_START_BLOCK);
  	printf("Wrote VCB in %d blocks starting at block %d.\n", totalVCBBlocks, VCB_START_BLOCK);
  	return blocksWritten;
}

VCB* getVCB() {
  	return VCBpointer; //pointer of our struct to the memory of the vcb
}

void initializeVCB() {
  	if(!AreWeInitialized) {
    		printf("initializeVCB: System not init!\n");
    		return;
  	}
  	printf("*************Initializing VCB*************\n");
  	sprintf(VCBpointer -> header, "%s", header); 
 
  //setting values for the vcb pointer
  	VCBpointer -> volumeSize = volumeSize;
  	VCBpointer -> blockSize = blockSize;
  	VCBpointer -> diskSizeBlocks = diskSizeBlocks;
  	VCBpointer -> vcbStartBlock = VCB_START_BLOCK;
  	VCBpointer -> totalVCBBlocks = totalVCBBlocks;
  	VCBpointer -> inodeStartBlock = inodeStartBlock;
 	 VCBpointer -> totalInodes = totalInodes;
  	VCBpointer -> totalInodeBlocks = totalInodeBlocks;
  	printf("initializeVCB: totalInodeBlocks %ld", VCBpointer->totalInodeBlocks);

  	VCBpointer -> freeMapSize = freeMapSize;
  	for(int i=0; i<freeMapSize; i++) {
    		VCBpointer->freeMap[i] = 0;
  	}
  for(int i=0; i<inodeStartBlock+totalInodeBlocks; i++) { //iterate to set bits into the freemap
    		setBitMap(VCBpointer -> freeMap, i);
  	}

  	printVCB();
  	writeVCB();
}

void initializeInodes() { 
  	if(!AreWeInitialized) {
    		printf("initInodes: System not initialized!\n");
    		return;
  	}

 	printf("*************Initializing inodes*************\n");

  	printf("Total disk blocks: %ld, total inodes: %d, total inode blocks: %d\n", diskSizeBlocks, totalInodes, totalInodeBlocks);

  	//Allocating and init the inodes: the first one is the root directory and we're using an id of 1

  	fdDir* inodes = calloc(totalInodeBlocks, blockSize);
  	inodes[0].id = 0;
  	inodes[0].inUse = 1;
  	inodes[0].type = I_DIR;
  	strcpy(inodes[0].name, "root");
 	strcpy(inodes[0].path, "/root");
  	inodes[0].lastAccessTime = time(0);
  	inodes[0].lastModificationTime = time(0);
  	inodes[0].numDirectBlockPointers = 0;

  //This is for handling all the other inodes (found some helpful info online to help getting inodes together properly)

  	for(int i = 1; i<totalInodes; i++) {
    		inodes[i].id = i;
    		inodes[i].inUse = 0;
    		inodes[i].type = I_UNUSED;
    		strcpy(inodes[i].parent, "");
    		strcpy(inodes[i].name, "");
    		inodes[i].lastAccessTime = 0;
    		inodes[i].lastModificationTime = 0;
    
    	for(int j=0; j<MAX_DATABLOCK_POINTERS; j++) {
      		inodes[i].directBlockPointers[j] = INVALID_DATABLOCK_POINTER;
    	}
    	inodes[i].numDirectBlockPointers = 0;
    
  	}

  //using a char pointer to write the inodes onto the disk
  	char* ptr = (char*) inodes;
  	LBAwrite(ptr, totalInodeBlocks, inodeStartBlock);
  	printf("Wrote %d inodes of size %ld bytes each starting at block %d.\n", totalInodes, sizeof(fdDir), inodeStartBlock);
  	free(inodes);
}

void printVCB() { //trying to get the VCB to print in hex and ASCII
  	int size = VCBpointer->totalVCBBlocks*(VCBpointer->blockSize);
  	int width = 16;
  	char* ptr = (char*)VCBpointer;
  	char ascii[width+1];
  	sprintf(ascii, "%s", "................");
  	printf("************* Printing VCB *************\n");
  	for(int i = 0; i<size; i++) {
    		printf("%02x ", ptr[i] & 0xff);
    		if(ptr[i]) {
      			ascii[i%width] = ptr[i];
    		}
    	if((i+1)%width==0&&i>0) {
      		ascii[i%width+1] = '\0';
      		printf("%s\n", ascii);
      		sprintf(ascii, "%s", ".......");
    	} 
	else if (i==size-1) {
      		for(int j=0; j<width-(i%(width-1)); j++) {
        	printf("   ");
      	}
      	ascii[i%width+1] = '\0';
      	printf("%s\n", ascii);
     	sprintf(ascii, "%s", ".......");
    		}
  	}
  	printf("VCB Size: %d bytes\n", size);
}

void init(uint64_t _volumeSize, uint64_t _blockSize) { //initializing all the values from the struct from earlier
  	printf("*************Initialize*************\n");
  	printf("volumeSize: %ld\n", volumeSize = _volumeSize);
  	printf("blockSize: %ld\n", blockSize = _blockSize);
  	printf("diskSizeBlocks: %ld\n", diskSizeBlocks = intDiv(volumeSize, blockSize));
  	printf("freeMapSize: %d\n", freeMapSize = diskSizeBlocks <= sizeof(uint32_t) * 8 ? 1 : diskSizeBlocks / sizeof(uint32_t) / 8);
  	printf("totalVCBBlocks: %d\n", totalVCBBlocks = intDiv(sizeof(VCB) + sizeof(uint32_t[freeMapSize]), blockSize));
  	printf("inodeStartBlock: %d\n", inodeStartBlock = VCB_START_BLOCK + totalVCBBlocks);
  	printf("totalInodes: %d\n", totalInodes = (diskSizeBlocks - inodeStartBlock) / (DATA_BLOCKS_PER_INODE + intDiv(sizeof(fdDir), blockSize)));
  	printf("totalInodeBlocks: %d\n", totalInodeBlocks = intDiv(totalInodes * sizeof(fdDir), blockSize));
  	printf("inodeSizeBytes: %ld\n", sizeof(fdDir));
  	printf("inodeSizeBlocks: %ld\n", intDiv(sizeof(fdDir), blockSize));

  	int vcbSize = allocateVCB(&VCBpointer);
 	 printf("VCB allocated in %d blocks.\n", vcbSize);

  	AreWeInitialized = 1;
  	printf("*************End Initialized*************\n");
}

int createVCB(char* volumeName, uint64_t _volumeSize, uint64_t _blockSize) {
  	printf("*************** Creating Volume.... ***************\n");
  	if(access(volumeName, F_OK) != -1) { //checking if volume already exists
    		printf("Cannot create volume '%s'. Volume already exists or has been initialized.\n", volumeName);
    		return -3;
  	}

  	uint64_t existingVolumeSize = _volumeSize;
  	uint64_t existingBlockSize = _blockSize;

  	int returnValue = startPartitionSystem (volumeName, &existingVolumeSize, &existingBlockSize);

  	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", volumeName, (ull_t)existingVolumeSize, (ull_t)existingBlockSize, returnValue);

  	if(!returnValue) {
    		init(_volumeSize, _blockSize);
    		initializeVCB();
    		initializeInodes();
  	}

  	closeVCB();
  	return returnValue;
}

void openVCB(char* volumeName) {
  	printf("************* Opening VCB *************\n");
  	if(!AreWeInitialized) {
    		uint64_t existingVolumeSize;
    		uint64_t existingBlockSize;

    	int returnValue =  startPartitionSystem(volumeName, &existingVolumeSize, &existingBlockSize);
    	if(!returnValue) {
      		init(existingVolumeSize, existingBlockSize);
      		readVCB();
      		printVCB();
    		}
  	}
	else {
    		printf("Error: Volume cannot be opened '%s'. Another volume is already opened.\n", volumeName);
  	}
}

void closeVCB() {
  	printf("************* Closing VCB *************\n");
  	if(AreWeInitialized) {
    		closePartitionSystem();
    		free(VCBpointer);
    		AreWeInitialized = 0;
  	} 
	else {
    		printf("Cannot close volume. Volume isn't open.\n");
  	}
}
