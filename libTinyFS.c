//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1;
FDTable fdTable;
diskInfo dInfo;


int tfs_mkfs(char *filename, int nBytes){
	//set empty blocks
	//set superblock
	fdTable = calloc(sizeof(FDTable), DEFAULT_DISK_SIZE / BLOCKSIZE);
	int disk = openDisk(filename, nBytes);
	if(disk < 0){
		return disk; //some error occurred
	}
	//new file if nBytes > 0
	unsigned char* initBytes = calloc(sizeof(char),BLOCKSIZE);
	if(nBytes > 0){
		int x = 1;

		//initialize superblock
		initBytes[0] = 1; //set block type
		initBytes[1] = 0x45;
		initBytes[2] = 0; //used block vector
		//initBytes[4] = //block number of root inode
		//number free blocks
		//nBytes/blockSize
		

		writeBlock(disk,0,initBytes);
		initBytes[0] = 4; //set block type
		for(x; x < nBytes / BLOCKSIZE; x++){
			initBytes[2] += 1; // assume no overflow

			writeBlock(disk,x,initBytes);
		}

	}else{// file already exists
		//NEED TO CHECK IF mountable -> how?
	}
	//don't need linked lists?
	dInfo = *calloc(sizeof(diskInfo),1); 
	dInfo.disk = disk;
	dInfo.filename = filename;
	return 0;

}
int tfs_mount(char *filename){
	//checks to see if file is mountable
	//anything else to load into mem?
	mountedDisk = dInfo.disk;
}
int tfs_unmount(void){
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
	file descriptor table (contains offset pointer)

*/

/*questions:
	how to implement error codes? -> print?
	is it unmountable if superblock does 
	not have magic number? 	-> superblock has 
	magic number, number free blocks is correct/where 
	they are supposed to be


	max number of blocks per disk? yes

	//should unmount close the disk?
	//max number of open disks?
*/