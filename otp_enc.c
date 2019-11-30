#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) { perror(msg); exit(0); } //error function used for reporting issues

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct socketaddir_in serverAddress;
    
}