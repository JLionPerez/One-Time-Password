#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int buffer_size(char* buffer[], char* file_name) {
	FILE* fp; 
	int b_length = 0; 
	size_t buffer_length = 0;

	fp = fopen(file_name, "r");
	getline(buffer, &buffer_length, fp);

	b_length = strlen(*buffer);

	fclose(fp);
	return b_length;
}

int main(int argc, char *argv[])
{
	int socketFD, charsWritten, charsRead;
	int port_number = atoi(argv[3]);
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char* plaintext_buffer;
	char* key_buffer;

	// Check usage & args
	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); }

	//checks if both files have the same size
	if(buffer_size(&plaintext_buffer, argv[1]) == buffer_size(&key_buffer, argv[2])) {
		//send info to server
		plaintext_buffer[strcspn(plaintext_buffer, "\n")] = '\0';
		key_buffer[strcspn(key_buffer, "\n")] = '\0';

		// printf("%s contents: %s\n", argv[1], plaintext_buffer);
		// printf("%s contents: %s\n", argv[2], key_buffer);
	}

	return 0;
}
