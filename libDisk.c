#include "libDisk.h"
#include "libTinyFS.h"

node *head = NULL; //head = neweset node

int openDisk(char *filename, int nBytes){
    //make sure same file not opened twice
    FILE* fd;
    if(nBytes){
        if(!(fd = fopen(filename,"w+"))){
            return -1;
            //error
        }else{
            head = addNode(head);
        }
    }else{
        if(!(fd = fopen(filename,"r+"))){
            return -1;
            //error
        }else{
            head = addNode(head);
        }
    }
        fclose(fd);
        head->filename = filename;
        return head->disk;
}

int readBlock(int disk, int bNum, void *block){
    FILE* fd = fopen(getDisk(head, disk),"r");
    fseek(fd, bNum * BLOCKSIZE, SEEK_SET);
    fread(block, BLOCK_SIZE, 1, fd);
    fclose(fd)
    return 0;
}

int writeBlock(int disk, int bNum, void *block){
    FILE* fd = fopen(getDisk(head, disk),"r+");
    fseek(fd, bNum * BLOCKSIZE, SEEK_SET);
    fwrite(block, BLOCK_SIZE, 1, fd);
    fclose(fd);
    //add recovery?
    return 0;
}

//to-do :
/*
    error-check in read/write
*/