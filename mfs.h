/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: mfs.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "b_io.h"
#include "VCB.h"

#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK
#define MAX_FILEPATH_SIZE 225
#define MAX_CHILDREN 64
#define	MAX_FILENAME_SIZE 20
#define MAX_DATABLOCK_POINTERS	64
#define INVALID_DATABLOCK_POINTER -1
#define INVALID_INODE_NAME "unused_inode"
#define MAX_DIRECTORY_DEPTH 10

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


struct fs_diriteminfo
	{
    ino_t d_ino; //inode number
    off_t d_off;   //offseet to next diritem
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};

typedef enum {I_FILE, I_DIR, I_UNUSED} InodeType; //inode in unix

char * getInodeTypeName(char * buf, InodeType type);

typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	uint64_t id; //holds index of inode in inodes array
	int inUse;
	InodeType type; //holds type of inode
	char parent [MAX_FILEPATH_SIZE]; //parent path
	char children[MAX_CHILDREN][MAX_FILENAME_SIZE]; //array holding name of children
	int numChildren; //number of children in a dir
	char name[MAX_FILENAME_SIZE]; //holds filename
	char path[MAX_FILEPATH_SIZE];
	time_t lastAccessTime; //time last accessed
	time_t lastModificationTime; //holds last modifieed time
	blkcnt_t sizeInBlocks; //size of file block (512)
	off_t sizeInBytes; //holds size of file in bytes
	int directBlockPointers[MAX_DATABLOCK_POINTERS];
	int numDirectBlockPointers; //number of elements in array pointers to data blocks
	unsigned short  d_reclen;		/*length of this record */
	unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
	} fdDir;


int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);
fdDir * fs_opendir(const char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdir
int fs_isFile(char * path);	//return 1 if file, 0 otherwise
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file

/* Added in some helpful routines     */

void fs_init();
void writeInodes();
void fs_close();

void parseFilePath(const char *pathname);
void printFilePath();

fdDir* getInode(const char *pathname);
fdDir* getFreeInode();
void printCurrentDirectoryPath();

int writeBufferToInode(fdDir* inode, char* buf, size_t bufSizeBytes, uint64_t blockNumber);

fdDir* createInode(InodeType type, const char* path);
int checkValidityOfPath();
int setParent(fdDir* parent, fdDir* child);
char* getParentPath(char* buf, const char* path);
int parentChild(fdDir* parent, fdDir* child); //parent child looping function

fdDir* getInodeByID(int id);

/* End of added routines */



struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	dev_t st_dev; //ID of device containing file
	ino_t st_ino; //inode #
	mode_t st_mode; //protection
	nlink_t st_nlink; //number of hard links
	uid_t st_uid; //User ID of owner
	gid_t st_gid; //group ID of owner
	dev_t st_rdev; //Device ID if its a special file
	};

int fs_stat(const char *path, struct fs_stat *buf);

#endif
