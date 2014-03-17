#ifndef TINYFS
#define TINYFS

#define MAX_FILE_NAME_LENGTH 8
#define BLOCKSIZE 256         		/* we already talked about this, it's part of the spec */
#define DEFAULT_DISK_SIZE 10240 	/* your program should use a 10240 Byte disk size giving you 40 blocks total. This is a default size. You must be able to support different possible values */
#define DEFAULT_DISK_NAME "tinyFSDisk" 	/* use this name for a default disk file name */
typedef int fileDescriptor;

#endif