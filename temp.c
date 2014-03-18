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

SSL* init(){
	SSL_CTX *sslContext = NULL;
	SSL *sslHandle = NULL;

	SSL_load_error_strings();
    SSL_library_init ();
	sslContext = SSL_CTX_new (SSLv23_client_method ());

	if (sslContext == NULL)
        ERR_print_errors_fp (stderr);

      // Create an SSL struct for the connection
    sslHandle = SSL_new (sslContext);
    if (sslHandle == NULL)
        ERR_print_errors_fp (stderr);

    return sslHandle;
}

// char *sslRead(){	
// 	const int BLOCKSIZE = 256;
//   	char *rc = NULL;
//   	int received, count = 0;
//   	char buffer[BLOCKSIZE];
//       while (1)
//         {
//           if (!rc)
//             rc = malloc (BLOCKSIZE * sizeof (char) + 1);
//           else
//             rc = realloc (rc, (count + 1) *
//                           BLOCKSIZE * sizeof (char) + 1);

//           received = SSL_read (c->sslHandle, buffer, readSize);
//           buffer[received] = '\0';

//           if (received > 0)
//             strcat (rc, buffer);

//           if (received < readSize)
//             break;
//           count++;
//         }

//   return rc;
// }

int main(){
    //SSL_CTX *sslContext
    	FILE* fd;

    	printf("as%d\n",fd = fopen("tinyFasdfasdfSDisk", "r+"));

    char *buff = calloc(sizeof(char),1024);
	SSL* mySSL= init();
	SSL_CIPHER* cipher = SSL_get_current_cipher(mySSL);
	//SSL_CIPHER_description(cipher,buff,1024);
	printf("asd\n");
	printf("%s\n",buff);
	int temp;
	char c;

}