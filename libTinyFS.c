//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1;
fileTableEntry *fileTable;
diskInfo dInfo;

unsigned char *blockBuffer;

fdList *openFDs;
int numOpenFiles = 0;

void setBit(int block){
	int err;
	unsigned char mask = 1, temp;
	mask = mask << 7;
	temp = mask;
	unsigned char* buff = calloc(sizeof(char), 1);
	if(err = readBlock(dInfo.disk,0,buff) < 0){
		return err;
	}
	mask = mask >> block % 8;
	if(buff[4 + (block / 8)] ^ mask == 0)
		printf("warning, this block is already set\n");
	buff[4 +(block / 8)] |= mask;
	if(err = writeBlock(dInfo.disk,0,buff) < 0){
		return err;
	}
	free(buff);
	return 0;
}

int clearBit(int block){
	int err;
	unsigned char mask = 1, temp;
	mask = mask << 7;
	temp = mask;
	unsigned char* buff = calloc(sizeof(char), 1);
	if(err = readBlock(dInfo.disk,0,buff) < 0){
		return err;
	}

	mask = mask >> block % 8;
	if(buff[4 + (block / 8)] & mask == 1)
		printf("warning, this block is already clear\n");
	buff[4 +(block / 8)] &= ~mask;
	if(err = writeBlock(dInfo.disk,0,buff) < 0){
		return err;
	}
	free(buff);
	return 0;
}


int tfs_mkfs(char *filename, int nBytes){
	int err;
	//set empty blocks
	//set superblock
	fileTable = calloc(sizeof(fileTableEntry), DEFAULT_DISK_SIZE / BLOCKSIZE);
	int disk = openDisk(filename, nBytes);
	if(disk < 0){
		return disk; //some error occurred
	}
	//new file if nBytes > 0
	unsigned char* initBytes = calloc(sizeof(char),BLOCKSIZE);
	if(nBytes > 0){
		int x;
		unsigned char mask = 0;
		//initialize superblock
		initBytes[0] = 1; //set block type
		initBytes[1] = 0x45;
		initBytes[2] = 0; //pointer to inode, will be set when first file created

		int x;
		for(x = 1; x < nBytes / BLOCKSIZE; x++){
			if(x % 8 == 0){
				mask = 1;
				mask = mask << 6;
			}
			initBytes[4+ (x / 8)] |= mask;
			mask = mask >> 1;
		}
		if(err = writeBlock(disk,0,initBytes) < 0){
			return err;
		};
		for(x = 4; x < 9; x++){
			initBytes[x] = 0;
		}
		//initBytes[4] = //block number of root inode
		//number free blocks
		//nBytes/blockSize
		
		initBytes[0] = 4; //set block type
		for(x; x < nBytes / BLOCKSIZE; x++){
			if(err = writeBlock(disk,x,initBytes) < 0){
				return err;
			}
		}
	}else{
		return 0;
	}
	free(initBytes);
	blockBuffer = calloc(BLOCKSIZE, sizeof(char));
	dInfo = *(diskInfo*)calloc(sizeof(diskInfo),1); 
	dInfo.disk = disk;
	dInfo.filename = filename;
	dInfo.size = nBytes / BLOCKSIZE;
	return 0;

}
int tfs_mount(char *filename){
	if(mountedDisk != -1){
		return MAXDISKS;
	}
	//checks to see if file is mountable
	int x, y, mask, disk = dInfo.disk;
	unsigned char *freeBlock = calloc(sizeof(char), BLOCKSIZE);
	unsigned char vector;
	readBlock(disk,0,blockBuffer);
		if(blockBuffer[0] != 1 && blockBuffer[1] != 0x45){
			free(freeBlock);
			return DISKCORRUPT; // disk corrupted
		}
	for(x = 1; x < dInfo.size; x++){
		readBlock(disk,x,freeBlock);
		if(freeBlock[1] != 0x45){
			free(freeBlock);
			return DISKCORRUPT; //disk corrupted
		}
		y = 4+ (x / 8);
		vector = blockBuffer[y];
		mask = 1 << (7 - (x % 8)); 
		if(freeBlock[0] == 4){//empty block
			if(vector & mask == 0){
				free(freeBlock);
				return DISKCORRUPT;
			}
		}else{
			if(vector & mask != 0){
				free(freeBlock);
				return DISKCORRUPT;
			}
		}
	}
	free(freeBlock);
	mountedDisk = dInfo.disk;
	return 0;
}


int tfs_unmount(void){
	free(fileTable);
	free(&dInfo);
	free(blockBuffer);
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
	myEntry.iNode = iNode;
	myEntry.offset = 0;
	myEntry.valid = 1;
	strcpy(myEntry.filename, name);
	
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