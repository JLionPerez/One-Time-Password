/*
* Title: Program 4 - OTP (keygen.c)
* Description: Generates key
* Author: Joelle Perez
* Date: 6 December 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {

    srand(time(NULL));

    int key_length = 0;
    char hold;

    key_length = atoi(argv[1]); //get length

    int i;
    for(i = 0; i < key_length; i++) { //get string
        hold = 'A' + (rand() % 27); //generate random letters

        if(hold == '[') { //if not a letter turn to space
            hold = ' ';
        }

        fprintf(stdout, "%c", hold); //print out char
    }
    fprintf(stdout,"\n");
}