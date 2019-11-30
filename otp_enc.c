#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) { //error function used for reporting issues
    perror(msg); 
    exit(0); 
} 

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct socketaddir_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[256];

    if(argc < 3) { //check usage & args
        frprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0); 
    }

    // set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear out the address struct
    portNumber = atoi(argv[3]); //get the port number, conver to an integer
    serverAddress.sin_family = AF_INET; //create a network-capable socket
    serverAddress.sin_port = htons(portNumber); //store the port number
    serverHostInfo = gethostbyname("localhost"); //convert the machine name into a special form of address

    if (serverHostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }

    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); //copy in the address

    //set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //create the socket

    if(socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    //connect to server
    if (connect(socketFD, (struct sockadder*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    //get input message from user
    printf("CLIENT: Enter ext to send to the server, and then hit enter: ");
    memset(buffer, '\0', sizeof(buffer)); //clear out the buffer array
    fgets(buffer, sizeof(buffer) - 1, stdin); //get input from the user, trunc to buffer - 1 chars, leaving \0
    buffer[strcspn(buffer, "\n")] = '\0'; //remove the trailing \n that fgets adds

    //send message to server
    charsWritten = send(socketFD, buffer, strlen(buffer), 0); //write to the server
    
    if(charsWritten < 0) {
        error("CLIENT: ERROR writing to socket");
    }

    if(charsWritten < strlen(buffer)) {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    //get return message from server
    memset(buffer, '\0', sizeof(buffer)); //clear out the buffer again for reuse
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); //read data from the socket, leaving \0 at end

    if(charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }

    printf("CLIENT: I recieved this from the server: \"%s\"\n", buffer);

    close(socketFD); //close the socket
    return 0;
}