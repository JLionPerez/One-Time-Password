#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

int child_counter = 0;

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

//encrypts plaintext into ciphertext
char encrypt(char p_char, char k_char) {
	char c_char;

	if(p_char == ' ') { p_char = '['; }
	if(k_char == ' ') { k_char = '['; }

	c_char = ((p_char - 65) + (k_char - 65)) % 27;
	c_char += 65;

	if(c_char == '[') { c_char == ' '; }

	return c_char;
}

//gets information for size, plaintext, and key
void get_info(char *plaintext_buffer, char *key_buffer, int *p_size, int establishedConnectionFD) {
	int charsRead;
	// gets the int from client and display it
	charsRead = recv(establishedConnectionFD, p_size, sizeof(int), 0);
	if (charsRead < 0) error("ERROR reading from socket");
	printf("SERVER: I received the buffer length from the client: \"%d\"\n", *p_size);
	fflush(stdout);

	// get the plaintext buffer
	memset(plaintext_buffer, '\0', *p_size);
	charsRead = 0;
	printf("PLAINTEXT - P_size is: %d\n", *p_size);
	fflush(stdout);
	while(charsRead < *p_size) {
		charsRead += recv(establishedConnectionFD, plaintext_buffer + charsRead, *p_size - charsRead, 0);
	}
	if (charsRead < 0) error("ERROR reading from socket");
	printf("SERVER: I received this plaintext from the client: \"%s\"\n", plaintext_buffer);
	fflush(stdout);

	// get the key buffer
	memset(key_buffer, '\0', *p_size);
	charsRead = 0;
	printf("KEY - P_size is: %d\n", *p_size);
	fflush(stdout);
	while(charsRead < *p_size) {
		charsRead += recv(establishedConnectionFD, key_buffer + charsRead, *p_size - charsRead, 0);
	}
	if (charsRead < 0) error("ERROR reading from socket");
	printf("SERVER: I received this key from the client: \"%s\"\n", key_buffer);
	fflush(stdout);	
}

void send_cipher(char *cipher_buffer, int *p_size, int establishedConnectionFD) {
	int charsWritten = 0;

	printf("P_size is: %d\n", *p_size);
	fflush(stdout);

	while(charsWritten < *p_size) {
		charsWritten += send(establishedConnectionFD, cipher_buffer + charsWritten, *p_size - charsWritten, 0);
	}
	if (charsWritten < 0) error("ERROR writing to socket");
}

void switch_pids(char *plaintext_buffer, char *key_buffer, char *cipher_buffer, int *p_size, int establishedConnectionFD) {
	pid_t spawnpid = fork();

	switch(spawnpid)
	{
		case -1: //error
				perror("error fork gone wrong");
				exit(1);
				break;
			
		case 0: //child

			//gets information for buffers
			get_info(plaintext_buffer, key_buffer, p_size, establishedConnectionFD);
			// printf("P_size: %d\n", *p_size);
			// fflush(stdout);

			//encrypt
			int i;
			for(i = 0; i < *p_size; i++) {
				cipher_buffer[i] = encrypt(plaintext_buffer[i], key_buffer[i]);
			}
			send_cipher(cipher_buffer, p_size, establishedConnectionFD); //sends back to server
			printf("SERVER: sending cipher: %s\n", cipher_buffer);
			fflush(stdout);

			close(establishedConnectionFD); // Close the existing socket which is connected to the client
			exit(0);
			break;
		
		default: //parent
			child_counter++;

			if(child_counter == 5) {
				waitpid(-1, NULL, 0);
				child_counter--;
				while (waitpid(-1, NULL, WNOHANG) > 0) child_counter--;
			}
    }
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, p_size;
	socklen_t sizeOfClientInfo;
	char buffer[256]; // message
	struct sockaddr_in serverAddress, clientAddress;
	char plaintext_buffer[99999];
	char key_buffer[99999];
	char cipher_buffer[p_size]; //same size as plaintext

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

	while(1) {

		if(child_counter > 5) {
			break;
		}

		// Accept a connection, blocking if one is not available until one connects
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");
		// child_counter++;

		//fork
		switch_pids(plaintext_buffer, key_buffer, cipher_buffer, &p_size, establishedConnectionFD);
		waitpid(-1, NULL, WNOHANG);

		printf("%dth children\n", child_counter);
		fflush(stdout);
	}

	// close(establishedConnectionFD); // Close the existing socket which is connected to the client
	//close(listenSocketFD); // Close the listening socket
	return 0; 
}