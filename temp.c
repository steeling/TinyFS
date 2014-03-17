#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
int main(){
int temp;
	FILE* fd;
	char c;
	fd = fopen("tinyFSDisk", "r");
	while(c = fgetc(fd) != -1){

		//if(c >= 'A' && c <= 'Z'){
			printf("%x",c);
		//}
		}
		printf("%x \n",c);
}