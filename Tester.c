/**************************************************************
* Class: CSC-415
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954)
* Student ID: 
* Project: Basic File System 
*
* File: Tester.c
*
* Description: Opens the volume and print out the inode. Trying to get everything to show up to test for debugging as well as  
*
**************************************************************/

#include "VCB.h"
#include "mfs.h"



int main(int argc, char* argv[]) {

 	if(argc<2) {
    	printf("Missing arguments. Try Tester followed by volumeName\n");
    	return 0;
  	}

  	char volumeName[MAX_FILENAME_SIZE];
  	strcpy(volumeName, argv[1]);
  	openVCB(volumeName);

  	fs_init();

  	for(int i=0; i<getVCB()->totalInodes; i++) {
    		fdDir* inode = getInodeByID(i);
    		printInode(inode);
  	}

  	fs_close();
  	closeVCB();

}

char inodeTypenameBuffer[24];

void printInode(fdDir* inode) {
  	printf("*************printInode*************\n");
  	printf("id: %ld\n", inode->id);
  	printf("type: %s\n", getInodeTypeName(inodeTypenameBuffer, inode->type));
  	printf("name: %s\n", inode->name);
  	printf("path: %s\n", inode->path);
  	printf("parent: %s\n", inode->parent);
  
  /* Print children. */
  	printf("children: ");
  	for(int i=0; i < inode->numChildren; i++) {
    		printf("%s ", inode->children[i]);
  	}
  	printf("\n");

  	printf("numChildren: %d\n", inode->numChildren);

  /* Print block pointers. */
  	printf("directBlockPointers: ");
  	for(int i=0; i < inode->numDirectBlockPointers; i++) {
    		printf("%d ", inode->directBlockPointers[i]);
  	}
 	 printf("\n");

  	printf("numDirectBlockPointers: %d\n", inode-> numDirectBlockPointers);
  	printf("sizeInBlocks: %ld\n", inode->sizeInBlocks);
  	printf("sizeInBytes: %ld\n", inode->sizeInBytes);
  	printf("lastAccessTime: %lld\n", (long long) inode->lastAccessTime);
  	printf("lastModificationTime: %lld\n", (long long) inode->lastModificationTime);
  	printf("*****************************\n");
}

