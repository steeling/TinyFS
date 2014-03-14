#ifndef LIBTINYFS
#define LIBTINYFS

/*error codes:
	success >= 0

*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_FILE_NAME_LENGTH 8
/* The default size of the disk and file system block */
#define BLOCKSIZE 256
/* Your program should use a 10240 Byte disk size giving you 40 blocks total. This is a default size. You must be able to support different possible values */
#define DEFAULT_DISK_SIZE 10240
/* use this name for a default disk file name */
#define DEFAULT_DISK_NAME “tinyFSDisk”
typedef int fileDescriptor;

/* Makes a blank TinyFS file system of size nBytes on the file specified by ‘filename’. This function should use the emulated disk library to open the specified file, and upon success, format the file to be mountable. This includes initializing all data to 0x00, setting magic numbers, initializing and writing the superblock and inodes, etc. Must return a specified success/error code. */
int tfs_mkfs(char *filename, int nBytes);
/* tfs_mount(char *filename) “mounts” a TinyFS file system located within ‘filename’. tfs_unmount(void) “unmounts” the currently mounted file system. As part of the mount operation, tfs_mount should verify the file system is the correct type. Only one file system may be mounted at a time. Use tfs_unmount to cleanly unmount the currently mounted file system. Must return a specified success/error code. */
int tfs_mount(char *filename);
int tfs_unmount(void);
/* Opens a file for reading and writing on the currently mounted file system. Creates a dynamic resource table entry for the file, and returns a file descriptor (integer) that can be used to reference this file while the filesystem is mounted. */
fileDescriptor tfs_openFile(char *name);
/* Closes the file, de-allocates all system/disk resources, and removes table entry */
int tfs_closeFile(fileDescriptor FD);
/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire file’s content, to the file system. Sets the file pointer to 0 (the start of file) when done. Returns success/error codes. */
int tfs_writeFile(fileDescriptor FD,char *buffer, int size);
/* deletes a file and marks its blocks as free on disk. */
int tfs_deleteFile(fileDescriptor FD);
/* reads one byte from the file and copies it to buffer, using the current file pointer location and incrementing it by one upon success. If the file pointer is already at the end of the file then tfs_readByte() should return an error and not increment the file pointer. */
int tfs_readByte(fileDescriptor FD, char *buffer);
/* change the file pointer location to offset (absolute). Returns success/error codes.*/
int tfs_seek(fileDescriptor FD, int offset);


typedef struct __attribute__ ((__packed__)){
	unsigned char firstINode;
	unsigned char freeListBitVector[5];
} superBlockFormat;

typedef struct __attribute__ ((__packed__)){
	unsigned char nextINode;
	unsigned char nextFileExtent;
	unsigned char fileSizeInBlocks;
	char filename[9];
} iNodeFormat;

typedef struct fileTableEntry {
	int valid;
	unsigned char iNode;
    unsigned long offset;
    int firstBlock;
    char filename[9];
} fileTableEntry;

typedef struct fdList {
	fileDescriptor fd;
	struct fdList *next;
} fdList;

extern fileTableEntry *fileTable;

extern fdList *openFDs;

extern int numOpenFiles;

typedef struct diskInfo{ // contains disks
	int disk;
	int size;
	char* filename;
} diskInfo;


int fileIsOpen(char *);

int fileOnFS(char *);

int spaceOnFS(void);

#endif