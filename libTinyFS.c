//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1;
fileTableEntry *fileTable;
diskInfo dInfo;


unsigned char *blockBuffer;

fdList *openFDs;
int numOpenFiles = 0;

int setBit(int block){
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
	printf("opened\n");
	unsigned char* initBytes = calloc(sizeof(char),BLOCKSIZE);
	if(nBytes > 0){
		int x;
		unsigned char mask = 0;
		//initialize superblock
		initBytes[0] = 1; //set block type
		initBytes[1] = 0x45;
		initBytes[2] = 0; //pointer to inode, will be set when first file created

		//int x;
		for(x = 1; x < nBytes / BLOCKSIZE; x++){
			if(x % 8 == 0){
				mask = 1;
				mask = mask << 7;
			}
			initBytes[4+ (x / 8)] |= mask;
			mask = mask >> 1;
		}
		mask = 1;
		mask = ~(mask << 7);
		initBytes[4] |= mask;
		printf("no write\n");

		if((err = writeBlock(disk,0,initBytes)) < 0){
			printf("%dern\n",err);
			return err;
		}
		printf("write\n");
		for(x = 4; x < 9; x++){
			initBytes[x] = 0;
		}
		//initBytes[4] = //block number of root inode
		//number free blocks
		//nBytes/blockSize
		
		initBytes[0] = 4; //set block type
		for(x= 0; x < nBytes / BLOCKSIZE; x++){
			if(err = writeBlock(disk,x,initBytes) < 0){
				return err;
			}
		}
	}else{
		return 0;
	}
	printf("close\n");
	free(initBytes);
	blockBuffer = calloc(BLOCKSIZE, sizeof(char));
	//dInfo = *(diskInfo*)calloc(sizeof(diskInfo),1); 
	dInfo.disk = disk;
	dInfo.filename = filename;
	dInfo.size = nBytes / BLOCKSIZE;
	return 0;

}
int tfs_mount(char *filename){
	if(mountedDisk != -1){
		return MAXDISKS;
	}
	int flag = openDisk(filename,0);
	if(flag < 0){
		return NODISK;
	}
	//checks to see if file is mountable
	int x, y, mask, disk = dInfo.disk;
	unsigned char *freeBlock = calloc(sizeof(char), BLOCKSIZE);
	unsigned char vector;
	printf("read att\n");
	int rdRtn = readBlock(disk,0,blockBuffer);
	if (rdRtn < 0) {
		return rdRtn;
	}
	printf("no read\n");
	if(blockBuffer[0] != 1 && blockBuffer[1] != 0x45){
		free(freeBlock);
		return DISKCORRUPT; // disk corrupted
	}
	for(x = 1; x < dInfo.size; x++){
		rdRtn = readBlock(disk,x,freeBlock);
		if (rdRtn < 0) {
			return rdRtn;
		}
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
	//free(&dInfo);
	free(blockBuffer);
	dInfo.disk = mountedDisk = -1;

}

fileDescriptor tfs_openFile(char *name) {
	fileDescriptor myFD;
	int iNode;
	//check strlen
	if (strlen(name) > MAX_FILE_NAME_LENGTH) {
		myFD = FILENAMETOOLONG;
	}
	//check if open
	else if (myFD = fileIsOpen(name)) {
		myFD = FILEOPEN;
	}
	//check if exists
	else if (iNode = fileOnFS(name)) {
		myFD = openFile(iNode, name);
	}
	//create it
	else {
		if (iNode = spaceOnFS()) {
			myFD = openFile(iNode, name);
			setBit(iNode);
			
			int nextINode = 0;
			
			int rdRtn = readBlock(dInfo.disk, 0, blockBuffer);
			if (rdRtn < 0) {
				return rdRtn;
			}
			superBlockFormat *super = (superBlockFormat *)blockBuffer;
			nextINode = super->firstINode;
			super->firstINode = iNode;
			int wtRtn = writeBlock(dInfo.disk, 0, blockBuffer);
			if (wtRtn < 0) {
				return wtRtn;
			}
			
			//set up iNode
			rdRtn = readBlock(dInfo.disk, iNode, blockBuffer);
			if (rdRtn < 0) {
				return rdRtn;
			}
			iNodeFormat *format = (iNodeFormat *)blockBuffer;
			format->blockType = 2;
			format->magicNumber = 0x45;
			format->nextINode = nextINode;
			format->nextFileExtent = 0;
			format->fileSize = 0;
			strcpy(format->filename, name);
			wtRtn = writeBlock(dInfo.disk, iNode, blockBuffer);
			if (wtRtn < 0) {
				return wtRtn;
			}
		}
		else {
			myFD = NODISKMEM;
		}
	}
	return myFD;
}

int tfs_closeFile(fileDescriptor FD){
	int rtn = 0;
	if (!(fileTable[FD].valid)) {
		rtn = FSBADFD;
	}
	else {
		fileTable[FD].valid = 0;
		fdList *newOpenFD = malloc(sizeof(fdList));
		newOpenFD->next = openFDs;
		newOpenFD->fd = FD;
		numOpenFiles--;
		rtn = 1;
	}
	return rtn;
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size){
	int rtn = 0;
	if (!(fileTable[FD].valid)) {
		rtn = FSBADFD;
	}
	else {
		int rdRtn = readBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
		if (rdRtn < 0) {
			return rdRtn;
		}
		iNodeFormat *iNode = (iNodeFormat*)blockBuffer;
		if (iNode->fileSize != 0) {
			int next = iNode->nextFileExtent;
			while (next) {
				rdRtn = readBlock(dInfo.disk, next, blockBuffer);
				if (rdRtn < 0) {
					return rdRtn;
				}
				fileExtentFormat *fileExtent = (fileExtentFormat *)blockBuffer;
				setBit(next);
				next = fileExtent->nextFileExtent;
			}
		}
		rdRtn = readBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
		if (rdRtn < 0) {
			return rdRtn;
		}
		iNode = (iNodeFormat*)blockBuffer;
		iNode->fileSize = size;
		int blocks = 0;
		int fileSize = size;
		int bufferOffset = 0;
		size += sizeof(iNodeFormat); //iNode header size in bytes
		while(size >= BLOCKSIZE) {
			size -= (BLOCKSIZE - 4);  //fileExtent header size in bytes
			blocks++;		
		}
		int *blocksUsed = malloc(blocks * sizeof(int));
		int i, j, check;
		for (i = 0; i < blocks; i++) {
			check = spaceOnFS();
			if (!check) {
				rtn = FSBIG;
				//NOT ENOUGH SPACE FOR FILE, WHAT ERROR?
				//clear the ones I've "taken" so far
				for (j = 0; j < i; j++ ) {
					setBit(blocksUsed[j]);
				}
			}
			else {
				blocksUsed[i] = check;
			}
		}
		int wtRtn;
		if (blocks == 0) { //fits in iNode
			memcpy(blockBuffer + sizeof(iNodeFormat), buffer, fileSize);
			wtRtn = writeBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
			if (wtRtn < 0) {
				return wtRtn;
			}
		}
		else {
			memcpy(blockBuffer + sizeof(iNodeFormat), buffer, BLOCKSIZE - sizeof(iNodeFormat));
			wtRtn = writeBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
			if (wtRtn < 0) {
				return wtRtn;
			}
			bufferOffset += BLOCKSIZE - sizeof(iNodeFormat);
			for (i = 0; i < blocks-1; i++) {
				rdRtn = readBlock(dInfo.disk, blocksUsed[i], blockBuffer);
				if (rdRtn < 0) {
					return rdRtn;
				}
				memcpy(blockBuffer + sizeof(fileExtentFormat), buffer + bufferOffset, BLOCKSIZE - sizeof(fileExtentFormat));
				bufferOffset += BLOCKSIZE - sizeof(fileExtentFormat);
			}
			rdRtn = readBlock(dInfo.disk, blocksUsed[i], blockBuffer);
			if (rdRtn < 0) {
				return rdRtn;
			}
			memcpy(blockBuffer + sizeof(fileExtentFormat), buffer + bufferOffset, fileSize - bufferOffset);
			wtRtn = writeBlock(dInfo.disk, blocksUsed[i], blockBuffer);
			if (wtRtn < 0) {
				return wtRtn;
			}
		}
		rtn = 1;
	}
	return rtn;
}

int tfs_deleteFile(fileDescriptor FD){
	int rtn = 0;
	if (!(fileTable[FD].valid)) {
		rtn = FSBADFD;
	}
	else {
		//mark its blocks as Free
		//fix iNode pointers
		fileTableEntry file = fileTable[FD];
		int rdRtn = readBlock(dInfo.disk, file.iNode, blockBuffer);
		if (rdRtn < 0) {
			return rdRtn;
		}	
		iNodeFormat *iFormat = (iNodeFormat*)blockBuffer;
		int nextINode = iFormat->nextINode;
		int next = iFormat->nextFileExtent;
		setBit(file.iNode);
		while (next) {
			rdRtn = readBlock(dInfo.disk, next, blockBuffer);
			if (rdRtn < 0) {
				return rdRtn;
			}
			fileExtentFormat *fileExtent = (fileExtentFormat *)blockBuffer;
			setBit(next);
			next = fileExtent->nextFileExtent;
		}		
		
		rdRtn = readBlock(dInfo.disk, 0, blockBuffer);
		if (rdRtn < 0) {
			return rdRtn;
		}
		superBlockFormat *super = (superBlockFormat *)blockBuffer;
		super->firstINode = nextINode;
		int wtRtn = writeBlock(dInfo.disk, 0, blockBuffer);
		if (wtRtn < 0) {
			return wtRtn;
		}
		
		rtn = tfs_closeFile(FD);
	}
	return rtn;
}

int tfs_readByte(fileDescriptor FD, char *buffer){
	int rtn = 0;
	fileTableEntry file = fileTable[FD];
	if (!file.valid) {
		rtn = FSBADFD;
	}
	else {
		int readLoc = file.offset;
		int blocks = 0;
		int rdRtn = readBlock(dInfo.disk, file.iNode, blockBuffer);
		if (rdRtn < 0) {
			return rdRtn;
		}
		iNodeFormat *iNode = (iNodeFormat *)blockBuffer;
		if (iNode->fileSize <= readLoc) {
			rtn = EOFREACH;
		}
		readLoc += sizeof(iNodeFormat); //iNode header size in bytes
		while(readLoc >= BLOCKSIZE) {
			readLoc -= (BLOCKSIZE - sizeof(fileExtentFormat));  //fileExtent header size in bytes
			blocks++;		
		}
		if (blocks) {
			readLoc += sizeof(fileExtentFormat); //fileExtent header size in bytes
			rdRtn = readBlock(dInfo.disk, file.iNode, blockBuffer);
			if (rdRtn < 0) {
				return rdRtn;
			}
			iNode = (iNodeFormat *)blockBuffer;
			int i, next = iNode->nextFileExtent;
			fileExtentFormat *fileExtent;
			for (i = 0; i < (blocks - 1); i++) {
				rdRtn = readBlock(dInfo.disk, next, blockBuffer);
				if (rdRtn < 0) {
					return rdRtn;
				}
				fileExtent = (fileExtentFormat *)blockBuffer;
				next = fileExtent->nextFileExtent;
			}
			memcpy(blockBuffer + readLoc, buffer, 1);
		}
		//data is in the iNode block
		else {
			rdRtn = readBlock(dInfo.disk, file.iNode, blockBuffer);
			if (rdRtn < 0) {
				return rdRtn;
			}
			memcpy(blockBuffer + readLoc, buffer, 1);
		}
		file.offset++;
		rtn = 1;
	}
	return rtn;
}
int tfs_seek(fileDescriptor FD, int offset){
	int rtn = 0;
	fileTableEntry file = fileTable[FD];
	if (!file.valid) {
		rtn = FSBADFD;
	}
	else {
		file.offset = offset;
		rtn = 1;
	}
	return rtn;
}

int openFile(int iNode, char *name) {
	fileDescriptor fd = 0;
	//check if no open space in table, then make new one
	if (openFDs == NULL) {
		fd = numOpenFiles;
		printf("in here\n");
	}
	else {
		printf("second\n");
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
			if (!strcmp(name, fileTable[i].filename)) {
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
	int rdRtn = readBlock(dInfo.disk, 0, blockBuffer);
	if (rdRtn < 0) {
		return rdRtn;
	}
	superBlockFormat *superBlock = (superBlockFormat *)blockBuffer;
	next = (int)(superBlock->firstINode);
	
	while (next) {
		rdRtn = readBlock(dInfo.disk, next, blockBuffer);
		if (rdRtn < 0) {
			return rdRtn;
		}
		iNodeFormat *iFormat = (iNodeFormat *)blockBuffer;
		if (!strcmp(iFormat->filename, name)) {
			iNode = next;
			break;
		}
		next = (int)(iFormat->nextINode);
	}
	return iNode;
}

int spaceOnFS() {
	printf("space1\n");
	int iNode = 0, i, j;
	int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
	int numBytes = 5; //numBlocks / 8;
	int rdRtn = readBlock(dInfo.disk, 0, blockBuffer);
	if (rdRtn < 0) {
		return rdRtn;
	}
	printf("%x, %x, %d,\n",blockBuffer[0], blockBuffer[1],temp);
	superBlockFormat *superBlock = (superBlockFormat *)blockBuffer;
	//first open on freeListBitVector, or zero
	printf("num: %d\n",numBytes);
	for (i = 0; i < numBytes; i++) { // <= instead of < ?
		//skip through if all full
			for(j = 0; j < 8; j++) {
				//printf("dump: %x\n",superBlock->freeListBitVector[i]);
				if (((superBlock->freeListBitVector[i] >> (7-j)) & 1)) {
					iNode = (i * 8) + j;
					clearBit(iNode);
					printf("node: %d\n",iNode);
					return iNode;
				}
			}
	}
						printf("node1: %d\n",iNode);

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