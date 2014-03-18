#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libTinyFS.h"
#include "TinyFS_errno.h"
#include "tinyFS.h"


int main() {
 	int numBlocks = 0, notGood = 1;
/*	printf("How many blocks do you want on your tinyFS? (Valid Range: 2-40)\n");
	while (notGood) {
		scanf("%d", &numBlocks);
		if (numBlocks <= 40 && numBlocks >= 2) {
			notGood = 0;
		}
		else {
			printf("Invalid input(Valid Range: 2-40), try again\n");
		}
	} */
	numBlocks = 40;
	printf("Making tfs\n");
	int numBytes = numBlocks * BLOCKSIZE;
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
	int rtn, testFD, i;
	int *fds =  calloc(100, sizeof(int));
	printf("Starting tests!\n\n");
	
	//Read and re-name
	printf("READ AND RE-NAME TEST\n");
	fds[5] = tfs_openFile("You?");
	fds[4] = tfs_openFile("Are");
	fds[3] = tfs_openFile("How");
	fds[2] = tfs_openFile("There");
	fds[1] = tfs_openFile("Hello");
	fds[0] = tfs_openFile("Why");
	tfs_writeFile(fds[0], fileBuffer, 50);
	tfs_writeFile(fds[1], fileBuffer, 100);
	tfs_writeFile(fds[2], fileBuffer, 150);
	tfs_writeFile(fds[3], fileBuffer, 200);
	tfs_writeFile(fds[4], fileBuffer, 250);
	tfs_writeFile(fds[5], fileBuffer, 300);
	tfs_readdir();
	printf("\n");
	tfs_rename(fds[3], "Did");
	tfs_rename(fds[4], "Rename");
	tfs_rename(fds[5], "Work?");
	tfs_readdir();
	for (i = 0; i < 6; i++) {
		tfs_deleteFile(fds[i]);
	}
	
	//TIMESTAMPS
	printf("\nTIMESTAMP TEST\n");
	testFD = tfs_openFile("timetest");
	struct timeval *timestamp = tfs_readFileInfo(testFD);
	wait(1);
	tfs_writeFile(testFD, fileBuffer, 10);
	wait(2);
	tfs_readByte(testFD, byteBuffer);
	printf("Created: %ld.%06ld\n", timestamp[0].tv_sec,timestamp[0].tv_usec);
	printf("Last Modified: %ld.%06ld\n", timestamp[1].tv_sec,timestamp[1].tv_usec);
	printf("Last Accessed: %ld.%06ld\n", timestamp[2].tv_sec,timestamp[2].tv_usec);
	tfs_deleteFile(testFD);
	
	//DELETE TEST
	//delete then create with same name
	printf("\nDELETE TEST\n");
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
	tfs_deleteFile(checkFD);
	
	//READ NOT WRITTEN TO TEST
	printf("\nREAD NOT WRITTEN TEST\n");
	testFD = tfs_openFile("ohai");
	rtn = tfs_readByte(testFD, byteBuffer);
	if (rtn != FILENOTWRIT) {
		perror("Not expected conditions on read to not written file");
	}
	tfs_deleteFile(testFD);
	
	tfs_unmount();
	printf("\nTesting done!\n");
}
