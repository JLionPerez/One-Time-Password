/*
* Title: Program 4 - OTP (otp_dec_d.c)
* Description: Decrypts ciphertext being sent by client using key
* Author: Joelle Perez
* Date: 7 December 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

int child_counter = 0;

/*
* Function name: error()
* Purpose: sends an error to report issues
* Arguments: const char *
* Returns: none
*/
void error(const char *msg) { perror(msg); exit(1); } 

/*
* Function name: decrypt()
* Purpose: decrypts ciphertext into plaintext
* Arguments: chars
* Returns: char
*/
char decrypt(char c_char, char k_char) {
	char p_char;

	if(c_char == ' ') { c_char = '['; } //be in scope of chars of 27
	if(k_char == ' ') { k_char = '['; }

    if(c_char < k_char) {
        p_char = (((c_char) - (k_char)) + 27) % 27; //making sure mod isn't negative
        p_char += 65;
    }

    else {
        p_char = ((c_char - 65) - (k_char - 65)) % 27; //run normally
        p_char += 65;
    }

    if(p_char == '[') { p_char = ' '; } //use spaces

	return p_char;
}

/*
* Function name: handshake()
* Purpose: checks if server is connected to the right client
* Arguments: int
* Returns: boolean
*/
bool handshake(int establishedConnectionFD) {
	char sends = 'd', receives;
	int ret;

	while((ret = send(establishedConnectionFD, &sends, 1, 0)) == 0) {} //send char
	if(ret < 0) exit(1);

	while((ret = recv(establishedConnectionFD, &receives, 1, 0)) == 0) {} //recieve char
	if(ret < 0) exit(1);

	if (receives == sends) { //compare if they're the same
		return true;
	}

	exit(2);
	return false;
}

/*
* Function name: get_info()
* Purpose: gets information for size, cipher, and key for later use
* Arguments: char *, int *, int
* Returns: none
*/
void get_info(char *cipher_buffer, char *key_buffer, int *p_size, int establishedConnectionFD) {
	int charsRead;

	// gets the int from client and display it
	charsRead = recv(establishedConnectionFD, p_size, sizeof(int), 0);
	if (charsRead < 0) error("SERVER: ERROR reading from socket");

	// get the plaintext buffer
	memset(cipher_buffer, '\0', *p_size);
	charsRead = 0;
	while(charsRead < *p_size) {
		charsRead += recv(establishedConnectionFD, cipher_buffer + charsRead, *p_size - charsRead, 0);
	}
	if (charsRead < 0) error("SERVER: ERROR reading from socket");

	// get the key buffer
	memset(key_buffer, '\0', *p_size);
	charsRead = 0;
	while(charsRead < *p_size) {
		charsRead += recv(establishedConnectionFD, key_buffer + charsRead, *p_size - charsRead, 0);
	}
	if (charsRead < 0) error("SERVER: ERROR reading from socket");	
}

/*
* Function name: send_plaintext()
* Purpose: send plaintext back to client
* Arguments: char *, int *, int
* Returns: none
*/
void send_plaintext(char *plaintext_buffer, int *p_size, int establishedConnectionFD) {
	int charsWritten = 0;

	while(charsWritten < *p_size) {
		charsWritten += send(establishedConnectionFD, plaintext_buffer + charsWritten, *p_size - charsWritten, 0);
	}
	if (charsWritten < 0) error("SERVER: ERROR writing to socket");
}

/*
* Function name: switch_pids()
* Purpose: forks for each process to decrypt each instance
* Arguments: char *, int *, int
* Returns: none
*/
void switch_pids(char *plaintext_buffer, char *key_buffer, char *cipher_buffer, int *p_size, int establishedConnectionFD) {
	pid_t spawnpid = fork();

	switch(spawnpid)
	{
		case -1: //error
				perror("SERVER: error fork gone wrong");
				exit(1);
				break;
			
		case 0: //child

			if(handshake(establishedConnectionFD)) {
				//gets information for buffers
				get_info(cipher_buffer, key_buffer, p_size, establishedConnectionFD);

				//decrypt, input each char into buffer
				int i;
				for(i = 0; i < *p_size; i++) {
					plaintext_buffer[i] = decrypt(cipher_buffer[i], key_buffer[i]);
				}
				send_plaintext(plaintext_buffer, p_size, establishedConnectionFD); //sends back to server
			}

			close(establishedConnectionFD); // Close the existing socket which is connected to the client
			exit(0);
			break;
		
		default: //parent
			child_counter++; //increase body count

			if(child_counter == 5) {
				waitpid(-1, NULL, 0);
				child_counter--; //bury evidence
				while (waitpid(-1, NULL, WNOHANG) > 0) child_counter--;
			}
    }
}

int main(int argc, char *argv[]) {
	int listenSocketFD, establishedConnectionFD, portNumber, p_size;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	char cipher_buffer[99999];
	char key_buffer[99999];
	char plaintext_buffer[p_size]; //same size as plaintext

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("SERVER: ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("SERVER: ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

	while(1) {

		// Accept a connection, blocking if one is not available until one connects
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("SERVER: ERROR on accept");

		//fork, continue to decryption
		switch_pids(plaintext_buffer, key_buffer, cipher_buffer, &p_size, establishedConnectionFD);
		waitpid(-1, NULL, WNOHANG);
	}
	
	return 0; 
}