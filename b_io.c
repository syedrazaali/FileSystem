/**************************************************************
* Class:  CSC-415-03 
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954)
* Student ID: 
* Project: File System Project
*
* File: b_io.c
*
* Description: b_io.c - Includes all the methods that open, read, write, and close files from the file system
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>	// needed for malloc
#include <string.h>	// needed for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20

uint64_t bufSize; //this can track runtime to getVCB()

typedef enum {NoWRITE, WRITE} fileMode; //track if a file is writeable or not writeable

typedef struct b_FCB {
	int linuxFD; //system File Descriptor
	char * buf; // our buffer for open files
	int index; //current pos of buffer
	int blockIndex; //tracks index of block in FCB
	int bufLen; //checks valid bytes in buffer
	fileMode mode; //when the file is open to write
	fdDir* inode; //inode pointer for a file
	int eof //end of file
} b_FCB;

b_FCB fcbArray[MAXFCBS]; //fcbArray will hold all the files
 
int startup = 0; //keep track of whether or not we're initialized

void b_init(){
	bufSize = getVCB()->blockSize;

	for(int i = 0; i < MAXFCBS; i++){
		fcbArray[i].linuxFD = -1;
		fcbArray[i].mode == NoWRITE;
	}
	
	startup = 1;

}

int b_getFCB(){ //routine gets the free file in the FCB array
	for(int i = 0; i < MAXFCBS; i++){
		if (fcbArray[i].linuxFD == -1){
			fcbArray[i].linuxFD == -2;
			return i;
		}
	}
	return(-1); //all threads are being used
}

int b_open(const char * filename, int flags){ //I tried to follow our previous assignments for this
	int fd;
	int returnFD;

	printf("b_open\n");

	if(startup == 0) b_init(); //start up the file system
	returnFD = b_getFCB(); //get our FD

	b_FCB* fcb;
	if(returnFD < 0){ //checking if theres an open fcb returned to us
		return -1; //return -1 if there is not
	}
	fcb = &fcbArray[returnFD];
	
	
	// cmd_cp2l, get inode and check if it exists (theres some sort of problem here but im not too sure why)
	fdDir* inode = getInode(filename);
	if(!inode){
		printf("b_open %s does not yet exist. \n", filename);


	//cmd_cp2fs handling:
	//check for create:
		if(flags & O_CREAT){
			printf("Creating %s\n", filename);
		
			inode = createInode(I_FILE, filename);
			char parentpath[FILENAME_MAX];
			getParentPath(parentpath, filename);
			fdDir* parent = getInode(parentpath);
			setParent(parent, inode);
			writeInodes();
		}
		else {
			return -1;
		}
	}
	
	fcb -> inode = inode;

	//release the mutex
	fcb -> buf = malloc(bufSize + 1); //changee bufSize + 1 and meemcpy to strcpy
	if(fcb -> buf == NULL){
		close(fd);
		fcb -> linuxFD = -1;
		return -1;
	}
	
	fcb -> bufLen = 0; //nothing was read yet
	fcb -> index = 0; //nothing was read yet
	
	printf("b_open: Opened file '%s' with fd %d\n", filename, fd);
	return(returnFD);
}

int b_write(int fd, char * buffer, int count){
	if(startup == 0){
		b_init(); //initialize
	}

	if((fd < 0) || (fd >= MAXFCBS)){
		return (-1);
	}

	if(fcbArray[fd].linuxFD == -1){
		return -1;
	}
	
	b_FCB* fcb = &fcbArray[fd];
	int freeSpace = bufSize - fcb -> index;

	printf("b_write: count=%d, fcb->index=%d, freeSpace=%d\n", count, fcb->index, freeSpace);

	fcb -> mode = WRITE; //write the last bytes in b_close

	/* check how many bytes can fit at end of buffer and for overflow.
	Copy chunks to buffer and update index */

	int copyLength = freeSpace > count ? count : freeSpace;
	int secondCopyLength = count  - copyLength;

	printf("Copying the first segment to fcb->buf+%d: %d bytes\n", fcb-> index, copyLength);
	memcpy(fcb -> buf + fcb -> index, buffer, copyLength);
	fcb-> index += copyLength;
	fcb -> index %= bufSize;

	//If theres a second chunk, write out the entire buffer then reseet buffer and copy second chunk to the buffer:

	if(secondCopyLength != 0){
		printf("Writing buffer to file descriptor. \n");
		uint64_t indexOfBlock = getFreeBlock();

		if(indexOfBlock == -1){
			printf("There is not enough free space!");
			return 0;
		}
		else {
			printf("\n\nFCB buff:\n\n%s", fcb->buf);
			writeBufferToInode(fcb->inode, fcb->buf, copyLength + secondCopyLength, indexOfBlock);
		}
		fcb -> index = 0;
		printf("Copying second segment to fcb ->buf+%d: %d bytes\n", fcb -> index, secondCopyLength);
		memcpy(fcb->buf+fcb->index, buffer+copyLength, secondCopyLength);
		fcb->index += secondCopyLength;
		fcb->index %= bufSize;
		}
		
		return copyLength + secondCopyLength;
}

int b_read(int fd, char * buffer, int count){
	struct b_FCB* fcb = &fcbArray[fd];
	int bytesRemaining = fcb -> bufLen - fcb -> index;

	printf("b_read: index = %d\n", fcb->index);
	printf("b_read: buflen = %d\n", fcb->bufLen);

	if(bytesRemaining > count){
		printf("Existing: Copying to %s from %ld for %d bytes.\n", buffer, fcb->buf + fcb -> index, count);
		memcpy(buffer, fcb -> buf + fcb -> index, count);
		fcb-> index += count;
		return count;
	}
	else {
		printf("Tail: Copying to %ld from %ld for %ld bytes.\n", buffer, fcb->buf+fcb->index, bytesRemaining);
    		memcpy(buffer, fcb->buf+fcb->index, bytesRemaining);
		
		if(fcb -> eof){
			printf("End of file reached. Goodbye.\n");
			fcb -> index += bytesRemaining;
			return bytesRemaining;
		}
		
		if(fcb -> blockIndex > fcb -> inode -> numDirectBlockPointers -1){
			printf("Block Index out of bounds.\n");
			return 0;
		}
		int blockNumber = fcb -> inode -> directBlockPointers[fcb -> blockIndex];
		LBAread(fcb -> buf, 1, blockNumber);
		fcb -> blockIndex ++;

		printf("Read new data.\n");
    		printf("***************************************\n");
    		printf("%s\n", fcb->buf);
    		printf("****************************************\n");
    		fcb->index = 0;
		int newBufferSize = fcb -> bufLen = strlen(fcb -> buf);
		printf("%d bytes read.\n", newBufferSize);

		if(newBufferSize < bufSize){
			fcb -> eof = 1;
		}
		int remainderofCount = count - bytesRemaining;
		int secondSegmentCount = newBufferSize > remainderofCount?remainderofCount:newBufferSize;

		printf("Second: Copying to %ld from %ld for %ld bytes.\n", buffer + bytesRemaining, fcb -> buf + fcb -> index, secondSegmentCount);
		memcpy(buffer + bytesRemaining, fcb -> buf + fcb -> index, secondSegmentCount);
		fcb-> index += secondSegmentCount;
		return bytesRemaining + secondSegmentCount;
	}
}

void b_close(int fd){
	b_FCB* fcb = &fcbArray[fd];
	printf("Closing file %d. \n", fd);

	if(fcb -> mode == WRITE && fcb -> index > 0){ //write the last bytes
		printf("File was in write mode. \n");
		uint64_t indexOfBlock = getFreeBlock();
		if(indexOfBlock == -1){
			printf("There is not enough free space");
			return;
		}
		else {
			printf("Writing remaining bytes... \n");
			writeBufferToInode(fcb -> inode, fcb-> buf, fcb -> index, indexOfBlock);
		}
	}
	close (fcb -> linuxFD); //close file handler
	free(fcb -> buf); //free associated buffer
	fcb -> buf  = NULL; 
	fcb -> linuxFD = -1; //return FCB to list of available FCB's
}




