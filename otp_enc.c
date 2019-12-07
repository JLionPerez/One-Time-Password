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
	int socketFD, charsWritten = 0, charsRead = 0;
	int portNumber = atoi(argv[3]);
	int p_size, k_size;
	bool p_flag, k_flag;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char* plaintext_buffer;
	char* key_buffer;
//	char cipher_buffer[p_size + 1];
	char buffer[256]; //message
    
	// Check usage & args
	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } 

	//GET BUFFERS READY
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
	if(p_size > k_size) { fprintf(stderr, "Error: key ‘%s’ is too short\n", argv[2]); exit(1); }

	//checks if all characters are valid
	if((!p_flag) || (!k_flag)) { fprintf(stderr, "otp_enc error: input contains bad characters\n"); exit(1); }

	//GET SENDING READY
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	//send length of plaintext (no need to send key length)
	charsWritten = send(socketFD, &p_size, sizeof(int), 0); //sends int
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	fflush(stdout);

	//send plaintext to server
	charsWritten = 0;
	while(charsWritten < p_size) {
		charsWritten += send(socketFD, plaintext_buffer + charsWritten, p_size - charsWritten, 0);
	}
	if (charsWritten < 0) error("CLIENT: ERROR plaintext writing to socket");
	if (charsWritten < p_size) { printf("CLIENT: WARNING: Not all data written to socket!\n"); fflush(stdout); }
	fflush(stdout);

	//send key to server
	charsWritten = 0;
	while(charsWritten < k_size) {
		charsWritten += send(socketFD, key_buffer + charsWritten, k_size - charsWritten, 0);
	}
	if (charsWritten < 0) error("CLIENT: ERROR key writing to socket");
	if (charsWritten < k_size) { printf("CLIENT: WARNING: Not all data written to socket!\n"); fflush(stdout); }
	fflush(stdout);

	// Get return message and cipher from server
	//memset(cipher_buffer, '\0', p_size);
//	memset(cipher_buffer, 0, sizeof(cipher_buffer));
	char* cipher_buffer = calloc(p_size + 1, 1);
	charsRead = 0;
	printf("P_size is this: %d\n", p_size);
	fflush(stdout);
	while(charsRead < p_size) {
		charsRead += recv(socketFD, cipher_buffer + charsRead, p_size - charsRead, 0);
	}
	if (charsRead < 0) error("ERROR reading from socket");
	printf("CLIENT: I received this cipher from the server: \"%s\"\n", cipher_buffer);
	fflush(stdout);

	// printf("P_size is this: %d\n", p_size);
	// fflush(stdout);
	//include newline in file if outputted

	close(socketFD); // Close the socket
	return 0;
}
