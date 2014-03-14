//make linked list of free blocks
#include "libTinyFS.h"

int mountedDisk = -1;
fileTableEntry *fileTable;
diskInfo dInfo;

unsigned char *blockBuffer;

fdList *openFDs;
int numOpenFiles = 0;

int tfs_mkfs(char *filename, int nBytes){
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
		//initialize superblock
		initBytes[0] = 1; //set block type
		initBytes[1] = 0x45;
		initBytes[2] = 0; //pointer to inode, will be set when first file created
		initBytes[4] -= 1;
		initBytes[4] = initBytes[4] >> 1;
		initBytes[5] -=1;
		initBytes[6] -=1;
		initBytes[7] -=1;
		initBytes[8] -=2;
		writeBlock(disk,0,initBytes);
		for(x = 4; x < 9; x++){
			initBytes[x] = 0;
		}
		//initBytes[4] = //block number of root inode
		//number free blocks
		//nBytes/blockSize
		
		initBytes[0] = 4; //set block type
		for(x; x < nBytes / BLOCKSIZE; x++){
			writeBlock(disk,x,initBytes);
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
	//checks to see if file is mountable
	int x, y, mask, disk = dInfo.disk;
	unsigned char *freeBlock = calloc(sizeof(char), BLOCKSIZE);
	unsigned char vector;
	readBlock(disk,0,blockBuffer);
		if(blockBuffer[0] != 1 && blockBuffer[1] != 0x45){
			free(freeBlock);
			return -1; // disk corrupted
		}
	for(x = 1; x < dInfo.size; x++){
		readBlock(disk,x,freeBlock);
		if(freeBlock[1] != 0x45){
			free(freeBlock);
			return -1; //disk corrupted
		}
		y = 4+ (x / 8);
		vector = blockBuffer[y];
		mask = 1 << (7 - (x % 8)); 
		if(freeBlock[0] == 4){//empty block
			if(vector & mask == 0){
				free(freeBlock);
				return -1;
			}
		}else{
			if(vector & mask != 0){
				free(freeBlock);
				return -1;
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
			readBlock(dInfo.disk, iNode, blockBuffer);
			iNodeFormat *format = (iNodeFormat *)blockBuffer;
			format->blockType = 2;
			format->magicNumber = 0x45;
			format->nextINode = 0;
			format->nextFileExtent = 0;
			format->fileSize = 0;
			strcpy(format->filename, name);
			writeBlock(dInfo.disk, iNode, blockBuffer);
		}
		else {
			myFD = -1; //ERRNO
		}
	}
	return myFD;
}

int tfs_closeFile(fileDescriptor FD){
	int rtn = 0;
	if (!(fileTable[FD].valid)) {
		//errno?
		rtn = -1;
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
	//if written to already, delete, open
	readBlock(dInfo.disk, fileTable[FD].iNode, blockBuffer);
	iNodeFormat *iNode = (iNodeFormat*)blockBuffer;
	if (iNode->fileSize != 0) {
	
	}
	iNode->fileSize = fileSize;
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
			//errno
			//Not enough open blocks
			for (j = 0; j < i; j++ ) {
				//clear j
				blocksUsed[j];
			}
			//un set the ones I grabbed NOT THE INODE
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
}

int tfs_deleteFile(fileDescriptor FD){
	int rtn = 0;
	if (!(fileTable[FD].valid)) {
		//errno?
		rtn = -1;
	}
	else {
		//mark its blocks as Free
		tfs_closeFile(FD);
	}
	return rtn;
}

int tfs_readByte(fileDescriptor FD, char *buffer){
	int rtn = 0;
	fileTableEntry *file = fileTable[FD];
	if (!file->valid) {
		//errno?
	}
	else {
		int readLoc = file->offset;
		int blocks = 0;
		readBlock(dInfo.disk, file->iNode, blockBuffer);
		iNode = (iNodeFormat *)blockBuffer;
		if (iNode->fileSize <= readLoc) {
			//errno
		}
		readLoc += sizeof(iNodeFormat); //iNode header size in bytes
		while(readLoc >= BLOCKSIZE) {
			readLoc -= (BLOCKSIZE - sizeof(fileExtentFormat));  //fileExtent header size in bytes
			blocks++;		
		}
		if (blocks) {
			readLoc += sizeof(fileExtentFormat); //fileExtent header size in bytes
			readBlock(dInfo.disk, file->iNode, blockBuffer);
			iNode = (iNodeFormat *)blockBuffer;
			int i, next = iNode->nextFileExtent;
			fileExtentFormat *fileExtent;
			for (i = 0; i < (blocks - 1); i++) {
				readBlock(dInfo.disk, next, blockBuffer);
				fileExtent = (fileExtentFormat *)blockBuffer;
				next = fileExtent->nextFileExtent;
			}
			memcpy(blockBuffer + readLoc, bufferm 1);
		}
		//data is in the iNode block
		else {
			readBlock(dInfo.disk, file->iNode, blockBuffer);
			memcpy(blockBuffer + readLoc, buffer, 1);
		}
		file->offset++;
	}
	return rtn;
}
int tfs_seek(fileDescriptor FD, int offset){
	int rtn = 0;
	fileTableEntry *file = fileTable[FD];
	if (!file->valid) {
		//errno?
		rtn = -1
	}
	else {
		file->offset = offset;
		rtn = 1;
	}
	return rtn;
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