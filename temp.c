#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
int main(){
	printf("%d\n",errno);
	int x;
	FILE* fd = fopen("asdfasdf", "r");
	printf("%d\n", x = errno);
	printf("%s\n",strerror(x));
	// unsigned char x = 0;
	// x -= 2;
	// //printf("%d\n", x);
	// int temp[4];
	// temp[0] = 1;
	// temp[2] = 2;

	// int temp2[8];
	// temp2[7] = 8;
	// memmove(temp2 + 2, temp, sizeof(int) * 3);
	// for(x = 0; x < 8; x++)
	// 		printf("%d\n", temp2[x]);

}