#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


int main(){

	int pid;
	if((pid = fork())){//parent
		int tempStatus;
		waitpid(pid, &tempStatus,0);
		printf("waited\n");
	}else {//child
		printf("execing\n");
		//int x = execl("/usr/bin/openssl",, NULL);
		//printf("wtf %d %s\n", x, strerror(errno));
		execl("/usr/bin/openssl", "enc", "-aes-256-cbc", "-salt", "-in", "output.txt", "-out", "output.enc", "-pass", "pass:j", NULL);
	}
	return 0;

}