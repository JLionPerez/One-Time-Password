#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(0); }

//put file contents into buffers and returns length of buffer
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

//checks if buffer has all valid chars including spaces and returns bool
bool valid_buffer(char* buffer, int length) {
	int i, counter = 0;

	for(i = 0; i < length; i++) {
		if((buffer[i] >= 'A') && (buffer[i] <= 'Z')) {
			counter++;
		}

		else if(buffer[i] == ' ') {
			counter++;
		}
	}

	if(counter == length) {
		return true;
	}

	return false;
}

int main(int argc, char *argv[])
{
	int socketFD, charsWritten, charsRead;
	int port_number = atoi(argv[3]);
	int p_size, k_size;
	bool p_flag, k_flag;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char* plaintext_buffer;
	char* key_buffer;

	// Check usage & args
	if (argc < 3) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); }

	//get the sizes including newline
	p_size = buffer_size(&plaintext_buffer, argv[1]);
	k_size = buffer_size(&key_buffer, argv[2]);

	//get rid of newlines
	plaintext_buffer[strcspn(plaintext_buffer, "\n")] = '\0';
	key_buffer[strcspn(key_buffer, "\n")] = '\0';
	p_size--; //reduce for no newline
	k_size--; 

	//checks for bad characters
	p_flag = valid_buffer(plaintext_buffer, p_size); //plaintext
	k_flag = valid_buffer(key_buffer, k_size);	//key

	//checks if key file is bigger or equal to plaintext
	if(p_size > k_size) {
		fprintf(stderr, "Error: key ‘%s’ is too short\n", argv[2]);
		exit(1);
	}

	//checks if all characters are valid
	if((!p_flag) || (!k_flag)) {
		fprintf(stderr, "otp_enc error: input contains bad characters\n");
		exit(1);
	}

	//send info to server
	printf("Sending buffers and size to server\n");

	return 0;
}
