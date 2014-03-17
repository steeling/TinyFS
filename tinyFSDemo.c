#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libTinyFS.h"
#include "TinyFS_errno.h"
#include "tinyFS.h"


int main() {
	int numBlocks = 0, notGood = 1;
	printf("How many blocks do you want on your tinyFS? (Valid Range: 2-40)\n");
	while (notGood) {
		scanf("%d ", &numBlocks);
		if (numBlocks <= 40 && numBlocks >= 2) {
			notGood = 0;
		}
		else {
			printf("Invalid input(Valid Range: 2-40), try again\n");
		}
	}
	int numBytes = blocks * BLOCKSIZE;
    /* try to mount the disk */
	if (tfs_mount(DEFAULT_DISK_NAME) < 0) {
		tfs_mkfs(DEFAULT_DISK_NAME, numBytes);	/* then make a new disk */
		if (tfs_mount(DEFAULT_DISK_NAME) < 0)	{
			perror ("failed to open disk");	/* then just exit */
			return 0;
		}
    }
	char *fileBuffer = calloc(sizeof(char), numBytes);
	char *byteBuffer = malloc(sizeof(char));
	int rtn, testFD;
	//DELETE TEST
	//delete then create with same name
	int deleteFD = tfs_openFile("deleteME");
	rtn = tfs_writeFile(deleteFD, fileBuffer, 12);
	if (rtn < 0) {
		perror("Write file failed for delete test");
	}
	rtn = tfs_deleteFile(deleteFD);
	if (rtn < 0) {
		perror("Delete file failed for delete test");
	}
	int checkFD = tfs_openFile("deleteME");
	if (checkFD < 0) {
		perror("Re-open failed for delete test");
	}

	//READ NOT WRITTEN TO TEST
	testFD = tfs_openFile("ohai");
	rtn = tfs_readByte(testFD, byteBuffer);
	if (rtn != FILENOTWRIT) {
		perror("Not expected conditions on read to not written file");
	}
}
