//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1;
//FDTable fdTable;
fileTableEntry *fileTable = NULL;
diskInfo dInfo;

char *blockBuffer = calloc(BLOCKSIZE, sizeof(char));

fdList *openFDs = NULL;
int numOpenFiles = 0;


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

fileDescriptor tfs_openFile(char *name) {
	fileDescriptor myFD;
	int iNode;
	//check strlen
	if (strlen(name) > MAX_FILE_NAME_LENGTH) {
		myFD = -1; //ERRNO?
	}
	//check if open
	else if (myFD = fileIsOpen(name)) {
		//Do nothing!
		//error?
	}
	//check if exists
	else if (iNode = fileOnFS(name)) {
		myFD = openFile(iNode, name);
	}
	//create it
	else {
		if (iNode = spaceOnFS()) {
			myFD = openFile(iNode, name);
			//set up iNode
		}
		else {
			myFD = -1; //ERRNO
		}
	}
	return myFD;
}

int tfs_closeFile(fileDescriptor FD){
	fileTable[FD].valid = 0;
	fdList *newOpenFD = malloc(sizeof(fdList));
	newOpenFD->next = openFDs;
	newOpenFD->fd = FD;
	numOpenFiles--;
}

int tfs_writeFile(fileDescriptor FD,char *buffer, int size){

}
int tfs_deleteFile(fileDescriptor FD){
	//call closeFile? everything there needs to be done
	//mark its blocks as Free
}
int tfs_readByte(fileDescriptor FD, char *buffer){

}
int tfs_seek(fileDescriptor FD, int offset){

}

int openFile(int iNode, char *name) {
	fileDescriptor fd = 0;
	//check if no open space in table, then make new one
	if (openFDs == NULL) {
		fileTableEntry *newTable = malloc(sizeof(fileTableEntry) * (numOpenFiles + 1));
		memcpy(newTable, fileTable, sizeof(fileTableEntry) * numOpenFiles);
		fileTable = newTable;
	}
	else {
		fd = openFDs->fd;
		openFDs = openFDs->next;
	}
	fileTableEntry myEntry = fileTable[fd];
	myEntry->iNode = iNode;
	myEntry->offset = 0;
	myEntry->valid = 1;
	strcpy(myEntry->filename, name);
	
	numOpenFiles++;
	return fd;
}

int fileIsOpen(char *name) {
	int fd = 0, i = 0;
	//check through fileTable
	while (i != numOpenFiles) {
		if (fileTable[i].valid == 1) {
			if (!strmp(name, fileTable[i].filename)) {
				fd = i;
				break;
			}
			i++;
		}
	}
	return fd;
}

int fileOnFS(char *name) {
	int iNode = 0, next = 0;
	//check through iNodes
	//return the iNode it is on
	
	//get the superblock
	readBlock(dInfo.disk, 0, blockBuffer);
	superBlockFormat *superBlock = (superBlockFormat *)blockBuffer;
	next = (int)(superBlock->firstINode);
	iNodeFormat *iNode;

	while (next) {
		readBlock(dInfo.disk, next, blockBuffer);
		iNode = (iNodeFormat *)blockBuffer;
		if (!strcmp(iNode->filename, name)) {
			iNode = next;
			break;
		}
		next = (int)(iNode->nextINode);
	}
	return iNode;
}

int spaceOnFS() {
	int iNode = 0, i, j;
	int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
	int numBytes = 5; //numBlocks / 8;
	readBlock(dInfo.disk, 0, blockBuffer);
	superBlockFormat *superBlock = (superBlockFormat *)blockBuffer;
	//first open on freeListBitVector, or zero
	for (i = 0; i < numBytes; i++) { // <= instead of < ?
		//skip through if all full
		if (superBlock->freeListBitVector[i] != 0xFF) {
			for(j = 0; j < 8; j++) {
				if (!((superBlock->freeListBitVector[i] >> j) & 1)) {
					iNode = (i * 8) + j;
					//set to not free anymore, could write a function 
					//as delete needs to do similar
					return iNode;
				}
			}
		}
		
	}
	return iNode;
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