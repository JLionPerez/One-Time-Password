#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {

    srand(time(NULL));

    int key_length = 0;
    char key_str[key_length];

    key_length = atoi(argv[1]);

    //strcpy(key_str, "hello");

    int i;
    char hold;
    for(i = 0; i < key_length; i++) {
        hold = 'A' + (rand() % 27);
        printf("%c", hold);
    }

    //printf("%s\n", key_str);

    //printf("%d\n", key_length);
}