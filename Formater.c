/**************************************************************
* Class:  CSC-415
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954)
* Student ID: N/A
* Project: Basic File System
*
* File: Formater.c
*
* Description: Formating the volumd of the disk
*
* 
* 
*
**************************************************************/


#include "mfs.h"

int main(int argc, char* argv[]){
	if(argc < 4){
		printf("Error: not all arguments passed!! Try Formater volumeName volumeSize blockSize\n");
		return 0;
	}
	
	char volumeName[MAX_FILENAME_SIZE]; //initialize volumeName with the max file size

	uint64_t volumeSize;
	uint64_t blockSize;

	strcpy(volumeName, argv[1]); //copy values of volumename to the first argument
	volumeSize = atoll(argv[2]);
	blockSize = atoll(argv[3]);

	createVCB(volumeName, volumeSize, blockSize); //init the volume control block
	openVCB(volumeName); //get our vcb running and our FS loaded
	closeVCB(); //close once its opened
	
	return 0;
}
	
