//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1;
diskNode* diskInfoHead = NULL;


int tfs_mkfs(char *filename, int nBytes){
	int disk = openDisk(filename, nBytes);
	if(disk < 0){
		return disk; //some error occurred
	}
	//new file if nBytes > 0
	unsigned char* initBytes = calloc(sizeof(char),BLOCKSIZE);
	if(nBytes > 0){
		int x = 1;

		//initialize file
		initBytes[0] = 1; //set block type
		initBytes[1] = 0x45;
		initBytes[2] = 1; //start of free block pointer
		//initBytes[4] = //block number of root inode

		writeBlock(disk,0,initBytes);

		initBytes[0] = 4; //set block type
		for(x; x < nBytes / BLOCKSIZE; x++){
			initBytes[2] += 1; // assume no overflow

			writeBlock(disk,x,initBytes);
		}

	}else{// file already exists
		//NEED TO CHECK IF mountable -> how?
	}
	diskInfoHead = addDiskNode(diskInfoHead);
	diskInfoHead->disk = disk;
	diskInfoHead->filename = filename;
	return 0;

}
int tfs_mount(char *filename){
//checks to see if file is mountable?
	diskNode* temp = diskInfoHead;
	while(temp){
		if(temp->filename = filename){
			mountedDisk = temp->disk;
			break;
		}
		temp = temp->next
	}
}
int tfs_unmount(void){
	//close disk?
	mountedDisk = -1;

}

fileDescriptor tfs_openFile(char *name){

}

int tfs_closeFile(fileDescriptor FD){

}

int tfs_writeFile(fileDescriptor FD,char *buffer, int size){

}
int tfs_deleteFile(fileDescriptor FD){

}
int tfs_readByte(fileDescriptor FD, char *buffer){

}
int tfs_seek(fileDescriptor FD, int offset){

}

/* to-do list
	what to do with error codes? -> print
*/

/*questions:
	how to implement error codes? -> print?
	is it unmountable if superblock does not have magic number? 
	max number of blocks per disk? 
		for free block pointer list.. if no would it be 
			better to do offset from current block?
	//check if correct type in mount? -> check first magic num?

	//should unmount close the disk?
	//max number of open disks?
*/