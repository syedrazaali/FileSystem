/**************************************************************
* Class:  CSC-415
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954)
* Student ID: N/A
* Project: Basic File System
*
* File: mfs.c
*
* Description: 
*	This file will initialize, track, and change our file system's inodes as well as track the basic shell functions. In essence, we intend to use this file as the core structure to our File System.
*	
*	
*
**************************************************************/

#include "mfs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> // Used for stat()
#include <sys/stat.h> // Used for stat()
#include <unistd.h>  // Used for stat()
#include <errno.h>

fdDir* inodes;

size_t NumberOfElementsInInodesArray = sizeof(inodes)/sizeof(inodes[0]); //keep track and calculate # of elements in inode array

void fs_init() {
	printf("*************fs_init*************\n");
	uint64_t totalBytes = getVCB() -> totalInodeBlocks * getVCB() -> blockSize;
	printf("totalInodeBlocks %ld, blockSize %ld\n", getVCB()->totalInodeBlocks, getVCB()->blockSize);
	printf("Allocating %ld bytes for inodes.\n", totalBytes); //trying to initialize all the bytes and blocks appropriately

	inodes = calloc(getVCB() -> totalInodeBlocks, getVCB() -> blockSize);
	printf("Inodes allocated at %p.\n", inodes);

	uint64_t blocksRead = LBAread(inodes, getVCB() -> totalInodeBlocks, getVCB() -> inodeStartBlock);
	printf("Loaded %ld blocks of inodes into cache.\n", blocksRead);
  
	if(blocksRead != getVCB()->totalInodeBlocks) {
		printf("Error: Not all inodes loaded into cache.\n");
		fs_close();
		exit(0);
  	}

	fs_setcwd("/root");
	printf("*****************************\n"); //putting this in to separate the outputs for each value so it's easier to read
}

void writeInodes() {
	printf("*************writeInodes*************\n");
	LBAwrite(inodes, getVCB() -> totalInodeBlocks, getVCB() -> inodeStartBlock);
	printf("*****************************\n");
}


char inodeTypeNames[3][64] = { "I_FILE", "I_DIR", "I_UNUSED" }; //following the enum i created in mfs.h

char* getInodeTypeName(char* buf, InodeType type) {
	strcpy(buf, inodeTypeNames[type]);
	return buf;
}

char cdPath[MAX_FILEPATH_SIZE]; //paths for our cd
char cdPathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int cdPathArraySize = 0;

//attempting to hold level for our paths
char reqFilePath[MAX_FILEPATH_SIZE];
char reqFilePathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int reqFilePathArraySize = 0;

/************************************************************Parsing File Path**********************************************************************************/


void parseFilePath(const char *pathname) { 
	printf("*************Parsing File*************\n");
	printf("Input: %s\n", pathname);

	reqFilePath[0] = '\0';
	reqFilePathArraySize = 0; //clear the value of the path and count

	char _pathname[MAX_FILEPATH_SIZE]; //making a nonchangeable copy of the filepath
	strcpy(_pathname, pathname);

	char* savePointer; //tokenizer
	char* token = strtok_r(_pathname, "/", &savePointer);

	int isAbsolute = pathname[0] == '/'; //options for pathname
	int isSelfRelative = !strcmp(token, ".");
	int isParentRelative = !strcmp(token, "..");


	if(token && !isAbsolute) {
		int maxLevel = isParentRelative ? cdPathArraySize - 1 : cdPathArraySize;
		for(int i=0; i<maxLevel; i++) {
			strcpy(reqFilePathArray[i], cdPathArray[i]);
			sprintf(reqFilePath, "%s/%s", reqFilePath, cdPathArray[i]);
			reqFilePathArraySize++;
    		}
  	}

	if(isSelfRelative || isParentRelative) {
		token = strtok_r(0, "/", &savePointer);
	}

	while(token && reqFilePathArraySize < MAX_DIRECTORY_DEPTH) {

		strcpy(reqFilePathArray[reqFilePathArraySize], token);
		sprintf(reqFilePath, "%s/%s", reqFilePath, token);

		printf("\t%s\n", token);
		reqFilePathArraySize++;
		token = strtok_r(0, "/", &savePointer);
	}

	printf("Output: %s\n", reqFilePath);
	printf("*****************************\n");
}

/************************************************************End of Parsing File Path**********************************************************************************/

/************************************************************Printing File Path**********************************************************************************/
void printFilePath() {

	for(int i = 0; i < reqFilePathArraySize; i++) {
		if(i < reqFilePathArraySize - 1) {
			printf("Directory %d: %s\n", i, reqFilePathArray[i]);
	} 
		else {
			printf("Filename: %s\n", reqFilePathArray[i]);
    		}

	}
}

/************************************************************Getting Inode**********************************************************************************/

fdDir* getInode(const char *pathname){
	printf("*************getInode*************\n");
	printf("Searching for path: '%s'\n", pathname);
	for (size_t i = 0; i < getVCB()-> totalInodes; i++) { //loop over inodes
		printf("\tInode path: '%s'\n", inodes[i].path);
		if (strcmp(inodes[i].path, pathname) == 0) { //finding requested node
			printf("*****************************\n");
			return &inodes[i];
		}
	}

	printf("Inode path '%s' does not exist. Please try again\n", pathname);

	printf("*****************************\n");
	return NULL; //if it doesn't exist return null

}

/************************************************************End of getting Inode**********************************************************************************/


/************************************************************getting free Inode**********************************************************************************/

fdDir* getFreeInode(){
	printf("*************getInode*************\n");
	fdDir* returnediNode;
	for (size_t i = 0; i < getVCB()->totalInodes; i++) { //search through inodes and reeturn first available inode
		if (inodes[i].inUse == 0) { // if inode is free we return 0
			inodes[i].inUse = 1; // update the node so that its used before returning it
			returnediNode = &inodes[i];
			printf("*****************************\n");
			return returnediNode;
      
		}
	}

	printf("*****************************\n");
	return NULL;
}

/************************************************************End of getting free Inode**********************************************************************************/


/************************************************************Creating Inode**********************************************************************************/

fdDir* createInode(InodeType type, const char* path){
	printf("*************CreateInode*************\n");
	fdDir* inode;
	char parentPath[MAX_FILEPATH_SIZE]; //return inode if successful and none if it can't be made
	fdDir* parentNode;

	time_t currentTime;
	currentTime = time(NULL); //get current time

	if (checkValidityOfPath() == 0){
		printf("*****************************\n");
		return NULL;
	}
	inode = getFreeInode(); //get next availablee inode
	getParentPath(parentPath, path);
	parentNode = getInode(parentPath); //assign parent to new inode
  	
//initializing values for inodes 
	inode->type = type;
	strcpy(inode->name , reqFilePathArray[reqFilePathArraySize - 1]);
	sprintf(inode->path, "%s/%s", parentPath, inode->name);
	inode->lastAccessTime = currentTime;
	inode->lastModificationTime = currentTime;

  //Setting parent stats
	if (!setParent(parentNode, inode)) {
		freeInode(inode);
		printf("Error with parent! No changes made.\n");
		printf("*****************************\n");
		return NULL;
	}
    
	printf("Inode successfully created for path '%s'.\n", path);   
	printf("*****************************\n");
	return inode;
}

/************************************************************End of creating free Inode**********************************************************************************/

/************************************************************Parent Child**********************************************************************************/

int parentHasChild(fdDir* parent, fdDir* child) { //dealing with parent that has child (return 1 if parent has child otherwise return 0)
	for( int i = 0; i < parent -> numChildren; i++ ) {
		if(!strcmp(parent -> children[i], child->name)) {
			return 1;
		}
	}
	return 0;
}

/************************************************************End of Parent Child**********************************************************************************/

/************************************************************Setting Parent**********************************************************************************/

int setParent(fdDir* parent, fdDir* child){
	printf("*************SetParent*************\n");
	if(parent->numChildren == MAX_CHILDREN) {
		printf("Folder '%s' can't hold any more children. \n", parent -> path);
		printf("*****************************\n");
		return 0;
	}
	if(parentHasChild(parent, child)) {
		printf("Folder '%s' already exists.\n", child->path);
		return 0;
	}

	strcpy(parent->children[parent->numChildren], child->name);
	parent->numChildren++;
	parent->lastAccessTime = time(0);
	parent->lastModificationTime = time(0);
	parent->sizeInBlocks += child->sizeInBlocks;
	parent->sizeInBytes += child->sizeInBytes;

	strcpy(child->parent, parent->path);
	sprintf(child->path, "%s/%s", parent->path, child->name);

  printf("Set parent of '%s' to '%s'.\n", child -> path, child -> parent);
  
	printf("*****************************\n");
	return 1;
}

/************************************************************End of setting Parent**********************************************************************************/

/************************************************************removefromparent**********************************************************************************/

int removeFromParent(fdDir* parent, fdDir* child) {
	printf("*************removefromParent*************\n");
	for(int i=0; i<parent->numChildren; i++) {
		if(!strcmp(parent->children[i], child->name)) { //i want int val returned not string
			strcpy(parent->children[i], "");
			parent->numChildren--;
			parent->sizeInBlocks -= child-> sizeInBlocks;
			parent->sizeInBytes -= child-> sizeInBytes;
			return 1;
		}
	}

	printf("Could not find child '%s' in parent '%s'.\n", child-> name, parent-> path);
	printf("*****************************\n");
	return 0;
}

/********************************************************Getting Parent Path***************************************************************************/

char* getParentPath(char* buf ,const char* path){
	printf("*************getParentPath*************\n");
	parseFilePath(path); //parse the file path to reach parent path
	char parentPath[MAX_FILEPATH_SIZE] = "";
	for(int i=0; i<reqFilePathArraySize - 1; i++) { //loop to reach 2nd to last elemetn
		strcat(parentPath, "/");
		strcat(parentPath, reqFilePathArray[i]);
	}

	strcpy(buf, parentPath);
	printf("Input: %s, Parent Path: %s\n", path, buf);
	printf("*****************************\n");
	return buf;
}

/********************************************************End of Getting Parent Path***************************************************************************/


/************************************************************Changing validation**********************************************************************************/


int checkValidityOfPath(){
	printf("*************checkValidityOfPath*************\n");
	printf("*****************************\n");
}

/********************************************************Start of Getting Inode ID Path***************************************************************************/

fdDir* getInodeByID(int id) {
	if(0 <= id < getVCB() -> totalInodes){ //forcing a return on the inode based on ID so we have some data on it
		return &inodes[id];
	} 
		else {
			return NULL;
	}
}

/********************************************************End of Getting Inode ID Path***************************************************************************/

/************************************************************writeBufferToInode directory**********************************************************************************/

int writeBufferToInode(fdDir* inode, char* buffer, size_t bufSizeBytes, uint64_t blockNumber) {
	printf("*************writebuffertoInode*************\n");
	int freeIndex = -1;
	for(int i=0; i<MAX_DATABLOCK_POINTERS; i++) { //making sure datablockpointer isn't full
		if(inode->directBlockPointers[i] == INVALID_DATABLOCK_POINTER) { //also making sure datablockpointer isn't invalid
			freeIndex = i;
			break;
		}
	}
	if(freeIndex == -1) { //return 0 if can't put datblock anywhere
		return 0;
	}

	LBAwrite(buffer, 1, blockNumber); //write the buf data to disk, update node and write it to disk
	inode->directBlockPointers[freeIndex] = blockNumber;
	setBitMap(getVCB()->freeMap, blockNumber);
	writeVCB();

//same stats as earlier for our inodes
	inode->numDirectBlockPointers++;
	inode->sizeInBlocks++;
	inode->sizeInBytes += bufSizeBytes;
	inode->lastAccessTime = time(0);
	inode->lastModificationTime = time(0);

	writeInodes();

	printf("*****************************\n");
	return 1;

}

/************************************************************end of Writebuffertoinode directory**********************************************************************************/


/************************************************************freeinode directory**********************************************************************************/

void freeInode(fdDir* node){
	printf("*************freeinode*************\n");
  	printf("Freeing inode: '%s'\n", node -> path);
  
	//init all values of nodes to 0 or null to free them
	node -> inUse = 0;
	node -> type = I_UNUSED;
	node -> name[0] = NULL;
	node -> path[0] = NULL;
	node -> parent[0] = NULL;
	node -> sizeInBlocks = 0;
	node -> sizeInBytes = 0;
	node -> lastAccessTime = 0;
	node -> lastModificationTime = 0;

	//freeing data blocks associated with the file
	if(node->type == I_FILE){
		for (size_t i = 0; i < node -> numDirectBlockPointers; i++) {
			int blockPointer = node -> directBlockPointers[i];
			clearBitMap(getVCB() -> freeMap, blockPointer);
		}
	}
  printf("*****************************\n");
  writeInodes(); //make sure the changes go to the disk

}

/************************************************************end of freeinode directory**********************************************************************************/


/************************************************************Close directory**********************************************************************************/

void fs_close() {
	printf("*************fs_closedir*************\n");
	free(inodes); //free all occuppied inodes then return
	printf("*****************************\n");
}
 
/************************************************************Making a directory**********************************************************************************/

int fs_mkdir(const char *pathname, mode_t mode) {
	printf("*************fs_mkdir*************\n");
	char parentPath[256] = "";
	parseFilePath(pathname); //parse filename and add info for the inode of the directory

	for (size_t i = 0; i < reqFilePathArraySize - 1; i++) { //setting the path to the appropriate position
		strcat(parentPath, "/");
		strcat(parentPath, reqFilePathArray[i]);
	}
  
	fdDir* parent = getInode(parentPath); //add info if necessary
	if (parent) {
		for (size_t i = 0; i < parent->numChildren; i++){
			if(strcmp(parent->children[i], reqFilePathArray[reqFilePathArraySize - 1])){ //checking if folder already exists
          printf("Folder already exists! Please try again or exit\n"); 
          printf("*****************************\n");
          return -1;
			}
		}
	} 
	else {
		printf("Parent '%s' does not exist! Please try again or exit\n", parentPath);
		printf("*****************************\n");
		return -1;
	}

	if( createInode(I_DIR, pathname)){
		writeInodes();
		printf("*****************************\n");
		return 0;
	}

	printf("Error, directory creation unsuccessful '%s'.\n", pathname);
	printf("*****************************\n");
	return -1;
}

/************************************************************ End of Making a directory**********************************************************************************/

/************************************************************Removing a directory**********************************************************************************/

int fs_rmdir(const char *pathname) {
	printf("*************fs_rmdir*************\n");
	fdDir* node = getInode(pathname);
	if(!node) {
		printf("%s does not exist.\n", pathname);
		return -1;
	}
	fdDir* parent = getInode(node->parent);
	if (node->type == I_DIR && node->numChildren == 0){ //node checking for removal
		removeFromParent(parent,node); //function to remove parent from node
		freeInode(node);
		printf("*****************************\n");
    		return 0;
	}
  	printf("*****************************\n");
  	return -1;
}

/************************************************************End of Removing a directory**********************************************************************************/


/************************************************************Open directory**********************************************************************************/

fdDir* fs_opendir(const char *fileName) {
  	printf("*************fs_opendir*************\n");
  	int checker = b_open(fileName, 0);
  	if(checker < 0) {
   		 printf("*****************************\n");
    		return NULL;
  	}
  	printf("*****************************\n");
  	return getInode(fileName);
}

/************************************************************End of Open directory**********************************************************************************/


int Counter = 0; //counter for our directory info
struct fs_diriteminfo directoryEntry;

struct fs_diriteminfo* fs_readdir(fdDir *dirp) {
  	printf("*************fs_readdir*************\n");
  	if(Counter == dirp->numChildren) {
    		Counter = 0;
    		return NULL;
  	}
  	char childPath[MAX_FILEPATH_SIZE];
  	sprintf(childPath, "%s/%s", dirp->path, dirp-> children[Counter]);
  	fdDir* child = getInode(childPath);

 	 directoryEntry.d_ino = child->id;
  	strcpy(directoryEntry.d_name, child->name);

  	Counter++;//increment the counter so that it reaches the next child

  	printf("*****************************\n");
  	return &directoryEntry;
}

int fs_closedir(fdDir *dirp) {
  	printf("*************fs_closedir*************\n");
  	printf("*****************************\n"); 
  	return 0;
}

char * fs_getcwd(char *buf, size_t size) {
  	printf("*************fs_getcwd*************\n");
 	 if(strlen(cdPath) > size) {
    		errno = ERANGE;
    		printf("*****************************\n");
    		return NULL;
  	}
  	strcpy(buf, cdPath);
  	printf("*****************************\n");
  	return buf;
}

/************************************************************Changing Directory**********************************************************************************/

int fs_setcwd(char *buf) {
  	printf("*************fs_setcwd*************\n");
  	parseFilePath(buf);

  	fdDir* inode = getInode(reqFilePath); //check if inode exists
  	if(!inode) {
    		printf("Directory '%s' does not exist!\n", reqFilePath);
    	printf("*****************************\n");
    	return 1;
  	}

  	/*null out the previous current working directory values*/
  	cdPath[0] = '\0';
  	cdPathArraySize = 0;

  	
  	for(int i=0; i<reqFilePathArraySize; i++) {
    		strcpy(cdPathArray[i], reqFilePathArray[i]);
    		sprintf(cdPath, "%s/%s", cdPath, reqFilePathArray[i]);
    		cdPathArraySize++;
  	}

  	printf("Set cwd to '%s'.\n", cdPath);
  	printf("*****************************\n");
  	return 0;
}

/********************************************************Print Current Directory Path***************************************************************************/

void printCurrentDirectoryPath() {
  	for(int i=0; i<cdPathArraySize; i++) {
    		if(i<cdPathArraySize-1) {
      			printf("Directory %d: %s\n", i, cdPathArray[i]);
    		} 
		else {
      			printf("Filename: %s\n", cdPathArray[i]);
    		}
  	}
}

/********************************************************End of Print Current Directory Path***************************************************************************/

/************************************************************fs_isFile**********************************************************************************/

int fs_isFile(char * path) { //checking to make ssure that the value given is infact a file
  	printf("*************fs_isFile*************\n");
  	fdDir* inode = getInode(path);
  	printf("*****************************\n");
  	return inode ? inode->type == I_FILE : 0;
}


/************************************************************fs_isDir**********************************************************************************/
int fs_isDir(char * path) {
  	printf("*************fs_isDir*************\n");
  	fdDir* inode = getInode(path);
  	printf("*****************************\n");
  	return inode ? inode->type == I_DIR : 0;
}

/************************************************************fs_delete**********************************************************************************/

int fs_delete(char* filePath) {
  	printf("*************fs_delete*************\n");
  	fdDir* fileNode = getInode(filePath); //getting the inode 
  	fdDir* parentNode = getInode(fileNode->parent); //parent
  	removeFromParent(parentNode, fileNode); //remove the child from its corresponding parent
  	freeInode(fileNode); //get rid of child inode so it doesn't show up when we reach the filesystem
 	printf("*****************************\n");
  	return 0;
}


/************************************************************fs_stat**********************************************************************************/

int fs_stat(const char *path, struct fs_stat *buf) {
  	printf("*************fs_stat*************\n");
  	fdDir* inode = getInode(path);
  	if(inode) { //setting all the stats for the inode:
    		buf->st_size = 999;
    		buf->st_blksize = getVCB()->blockSize;
    		buf->st_blocks = 2;
    		buf->st_accesstime = 1;
    		buf->st_modtime = 1;
    		buf->st_createtime = 1;
    		printf("*****************************\n");
    		return 1;
  	}
  	printf("*****************************\n");
  	return 0;
}
