#include "libDisk.h"



typedef struct node{
    int disk;
    char* filename;
    struct node* next;
}node;

node* addNode(node* base){//fix this up
    node* temp;

    if(base != NULL){
        temp = base;
        base = calloc(sizeof(node),1);
        base->disk = temp->disk + 1;
        base->next = temp;
    }else{
        base = calloc(sizeof(node),1);
        base->disk = 0;
    }
    return base;
}

char* getDisk(node* head, int num){
    while(head && head->next && head->disk != num){
        head = head->next;
    }
    return head->filename;
}

node *head = NULL; //head = neweset node

int openDisk(char *filename, int nBytes){
    FILE* fd;
    if(nBytes){
        if(!(fd = fopen(filename,"w+"))){
            return DISKNOCREAT;
        }else{
            head = addNode(head);
        }
    }else{
        if(!(fd = fopen(filename,"r+"))){
            return DISKNOREAD;
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
    FILE* fd;
    if(!(fd = fopen(getDisk(head, disk),"r"))){
        return DISKNOREAD;
    }
    fseek(fd, bNum * BLOCKSIZE, SEEK_SET);
    if(fread(block, BLOCKSIZE, 1, fd) < BLOCKSIZE){
        return INCOMPREAD;
    }
    fclose(fd);
    return 0;
}

int writeBlock(int disk, int bNum, void *block){
    FILE* fd;
        int temp;
    if(!(fd = fopen(getDisk(head, disk),"r+"))){
        return DISKNOREAD;
    }

    fseek(fd, bNum * BLOCKSIZE, SEEK_SET);
    if((temp = fwrite(block, BLOCKSIZE, 1, fd)) < BLOCKSIZE){
        printf("asdfasdf  %s  %d\n",strerror(errno),temp);
        return INCOMPWRIT;
    }
    fclose(fd);
    //add recovery?
    return 0;
}

//to-do :
/*
    error-check in read/write
*/