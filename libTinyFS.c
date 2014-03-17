//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1, totalDiskBytes;
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
	unsigned char* buff = calloc(BLOCKSIZE, 1);
	if(err = readBlock(dInfo.disk,0,buff) < 0){
			free(buff);

		return err;
	}
	mask = mask >> block % 8;
	if(buff[4 + (block / 8)] & mask == 1)
		printf("warning, this block is already set\n");
	buff[4 +(block / 8)] |= mask;
	if(err = writeBlock(dInfo.disk,0,buff) < 0){
			free(buff);
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
	unsigned char* buff = calloc(BLOCKSIZE, 1);
	if(err = readBlock(dInfo.disk,0,buff) < 0){
		free(buff);
		return err;
	}

	mask = mask >> block % 8;
	if(buff[4 + (block / 8)] & mask == 0)
		printf("warning, this block is already clear\n");
	buff[4 +(block / 8)] &= ~mask;
	if(err = writeBlock(dInfo.disk,0,buff) < 0){
		free(buff);
		return err;
	}
	free(buff);
	return 0;
}

int tfs_mkfs(char *filename, int nBytes){
	totalDiskBytes = nBytes;
	int err;
	//set empty blocks
	//set superblock
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

		if((err = writeBlock(disk,0,initBytes)) < 0){
			return err;
		}
		for(x = 4; x < 9; x++){
			initBytes[x] = 0;
		}
		//initBytes[4] = //block number of root inode
		//number free blocks
		//nBytes/blockSize
		
		initBytes[0] = 4; //set block type
		for(x= 1; x < nBytes / BLOCKSIZE; x++){
			if(err = writeBlock(disk,x,initBytes) < 0){
				return err;
			}
		}
	}else{
		return 0;
	}
	free(initBytes);
	return 0;

}



int tfs_mount(char *filename){
	if(mountedDisk != -1){
		return MAXDISKS;
	}
	int disk = openDisk(filename,0);
	if(disk < 0){
		return NODISK;
	}

	fileTable = calloc(sizeof(fileTableEntry), DEFAULT_DISK_SIZE / BLOCKSIZE);
	blockBuffer = calloc(BLOCKSIZE, sizeof(char));
	//dInfo = *(diskInfo*)calloc(sizeof(diskInfo),1); 
	dInfo.disk = disk;
	dInfo.filename = filename;
	dInfo.size = totalDiskBytes / BLOCKSIZE;

	printf("pass open\n");
	//checks to see if file is mountable
		int temp = 0;

		printf("hello: %d\n", temp++);

	int x, y, mask;
	unsigned char *freeBlock = calloc(sizeof(char), BLOCKSIZE);
	unsigned char vector;
		printf("hello: %d\n", temp++);

	readBlock(disk,0,blockBuffer);
	printf("hello: %d\n", temp++);
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
	printf("fini");
	return 0;
}


int tfs_unmount(void){
	free(fileTable);
	//free(&dInfo);
	free(blockBuffer);
	dInfo.disk = mountedDisk = -1;

}

fileDescriptor tfs_openFile(char *name) {
	printf("Here\n");
	fileDescriptor myFD;
	int iNode;
	//check strlen
	if (strlen(name) > MAX_FILE_NAME_LENGTH) {
		myFD = FILENAMETOOLONG;
	}
	//check if open
	else if (myFD = fileIsOpen(name)) {
		printf("here\n");

		myFD = FILEOPEN;
	}
	//check if exists
	else if (iNode = fileOnFS(name)) {
		printf("inode: %d\n",iNode);
		myFD = openFile(iNode, name);
	}
	//create it
	else {
		if (iNode = spaceOnFS()) {
			myFD = openFile(iNode, name);
			int nextINode = 0;
			
			readBlock(dInfo.disk, 0, blockBuffer);
			superBlockFormat *super = (superBlockFormat *)blockBuffer;
			nextINode = super->firstINode;
			printf("NODE: %d\n", nextINode);
			super->firstINode = iNode;
			writeBlock(dInfo.disk, 0, blockBuffer);
			
			//set up iNode
			free(blockBuffer);
			blockBuffer = calloc(BLOCKSIZE, sizeof(char));

			iNodeFormat *format = (iNodeFormat *)blockBuffer;
			format->blockType = 2;
			format->magicNumber = 0x45;
			format->nextINode = nextINode;
			format->nextFileExtent = 0;
			format->fileSize = 0;
			strcpy(format->filename, name);
			writeBlock(dInfo.disk, iNode, blockBuffer);
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
		readBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
		iNodeFormat *iNode = (iNodeFormat*)blockBuffer;
		if (iNode->fileSize != 0) {
			int next = iNode->nextFileExtent;
			while (next) {
				readBlock(dInfo.disk, next, blockBuffer);
				fileExtentFormat *fileExtent = (fileExtentFormat *)blockBuffer;
				setBit(next);
				next = fileExtent->nextFileExtent;
			}
		}
		readBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
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

		if (blocks == 0) { //fits in iNode
			memcpy(blockBuffer + sizeof(iNodeFormat), buffer, fileSize);
			writeBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
		}
		else {
			memcpy(blockBuffer + sizeof(iNodeFormat), buffer, BLOCKSIZE - sizeof(iNodeFormat));
			writeBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
			bufferOffset += BLOCKSIZE - sizeof(iNodeFormat);
			for (i = 0; i < blocks-1; i++) {
				readBlock(dInfo.disk, blocksUsed[i], blockBuffer);
				memcpy(blockBuffer + sizeof(fileExtentFormat), buffer + bufferOffset, BLOCKSIZE - sizeof(fileExtentFormat));
				bufferOffset += BLOCKSIZE - sizeof(fileExtentFormat);
			}
			readBlock(dInfo.disk, blocksUsed[i], blockBuffer);
			memcpy(blockBuffer + sizeof(fileExtentFormat), buffer + bufferOffset, fileSize - bufferOffset);
			writeBlock(dInfo.disk, blocksUsed[i], blockBuffer);
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
		readBlock(dInfo.disk, file.iNode, blockBuffer);
		iNodeFormat *iFormat = (iNodeFormat*)blockBuffer;
		int nextINode = iFormat->nextINode;
		int next = iFormat->nextFileExtent;
		setBit(file.iNode);
		while (next) {
			readBlock(dInfo.disk, next, blockBuffer);
			fileExtentFormat *fileExtent = (fileExtentFormat *)blockBuffer;
			setBit(next);
			next = fileExtent->nextFileExtent;
		}		
		
		readBlock(dInfo.disk, 0, blockBuffer);
		superBlockFormat *super = (superBlockFormat *)blockBuffer;
		super->firstINode = nextINode;
		writeBlock(dInfo.disk, 0, blockBuffer);
		
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
		readBlock(dInfo.disk, file.iNode, blockBuffer);
		iNodeFormat *iNode = (iNodeFormat *)blockBuffer;
		if (iNode->fileSize <= readLoc) {
			return rtn = EOFREACH;
		}
		readLoc += sizeof(iNodeFormat); //iNode header size in bytes
		while(readLoc >= BLOCKSIZE) {
			readLoc -= (BLOCKSIZE - sizeof(fileExtentFormat));  //fileExtent header size in bytes
			blocks++;		
		}
		if (blocks) {
			readLoc += sizeof(fileExtentFormat); //fileExtent header size in bytes
			readBlock(dInfo.disk, file.iNode, blockBuffer);
			iNode = (iNodeFormat *)blockBuffer;
			int i, next = iNode->nextFileExtent;
			fileExtentFormat *fileExtent;
			for (i = 0; i < (blocks - 1); i++) {
				readBlock(dInfo.disk, next, blockBuffer);
				fileExtent = (fileExtentFormat *)blockBuffer;
				next = fileExtent->nextFileExtent;
			}
			memcpy(blockBuffer + readLoc, buffer, 1);
		}
		//data is in the iNode block
		else {
			readBlock(dInfo.disk, file.iNode, blockBuffer);
			memcpy(blockBuffer + readLoc, buffer, 1);
		}
		file.offset++;
		rtn = 1;
	}
	fileTable[FD] = file;
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
	}
	else {
		fd = openFDs->fd;
		openFDs = openFDs->next;
	}
	(fileTable[fd]).iNode = iNode;
	(fileTable[fd]).offset = 0;
	(fileTable[fd]).valid = 1;
	strcpy((fileTable[fd]).filename, name);
	
	numOpenFiles++;
	return fd;
}

int fileIsOpen(char *name) {
	printf("in open\n");
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
	printf("returned\n");
	return fd;
}

int fileOnFS(char *name) {
	int iNode = 0, next = 0;
	//check through iNodes
	//return the iNode it is on
	
	//get the superblock
	readBlock(dInfo.disk, 0, blockBuffer);
	printf("read super\n");
	superBlockFormat *superBlock = (superBlockFormat *)blockBuffer;
	next = (int)(superBlock->firstINode);
			printf("here %d\n",next);

	while (next) {

		readBlock(dInfo.disk, next, blockBuffer);
		iNodeFormat *iFormat = (iNodeFormat *)blockBuffer;
		//printf("what's here %x %x %x\n",blockBuffer[0], blockBuffer[1], blockBuffer[2]);

		if (!strcmp(iFormat->filename, name)) {
			iNode = next;
			break;
		}
		next = (int)(iFormat->nextINode);
	}

	return iNode;
}

int spaceOnFS() {
	int iNode = 0, i, j;
	int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
	int numBytes = 5; //numBlocks / 8;
	int temp = readBlock(dInfo.disk, 0, blockBuffer);
	superBlockFormat *superBlock = (superBlockFormat *)blockBuffer;
	//first open on freeListBitVector, or zero
	for (i = 0; i < numBytes; i++) { // <= instead of < ?
		//skip through if all full
			for(j = 0; j < 8; j++) {
				if (((superBlock->freeListBitVector[i] >> (7-j)) & 1)) {
					iNode = (i * 8) + j;
					clearBit(iNode);
					return iNode;
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