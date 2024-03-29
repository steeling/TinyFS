#ifndef LIBTINYFS
#define LIBTINYFS

/*error codes:
	success >= 0

*/
#include "TinyFS_errno.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#include "tinyFS.h"



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

/* renames a file.  New name should be passed in. */
int tfs_rename(fileDescriptor FD, char *newName);
 /* lists all the files and directories on the disk */
int tfs_readdir();
/* returns the file’s creation time */
struct timeval *tfs_readFileInfo(fileDescriptor FD);

int setCreated(fileDescriptor FD);
int setLastModified(fileDescriptor FD);
int setLastAccess(fileDescriptor FD);

typedef struct __attribute__ ((__packed__)){
	unsigned char blockType;
	unsigned char magicNumber;
	unsigned char firstINode;
	unsigned char filler;
	unsigned char freeListBitVector[5];
} superBlockFormat;

typedef struct __attribute__ ((__packed__)){
	unsigned char blockType;
	unsigned char magicNumber;
	unsigned char nextINode;
	unsigned char nextFileExtent;
	unsigned int fileSize;
	char filename[9];
	struct timeval created;
	struct timeval lastModified;
	struct timeval lastAccess;
} iNodeFormat;

typedef struct {
	unsigned char blockType;
	unsigned char magicNumber;
	unsigned char nextFileExtent;
	unsigned char filler;
}	fileExtentFormat;

typedef struct fileTableEntry {
	int valid;
	unsigned char iNode;
    unsigned int offset;
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
	char* realName;
} diskInfo;


int fileIsOpen(char *);

int fileOnFS(char *);

int spaceOnFS(void);

#endif