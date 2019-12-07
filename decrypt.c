#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

char decrypt(char c_char, char k_char) {
	char p_char;

	if(c_char == ' ') { c_char = '['; }
	if(k_char == ' ') { k_char = '['; }

    if(c_char < k_char) {
        p_char = (((c_char) - (k_char)) + 27) % 27;
        p_char += 65;
    }

    else {
        p_char = ((c_char - 65) - (k_char - 65)) % 27;
        p_char += 65;
    }

    if(p_char == '[') { p_char = ' '; }

	return p_char;
}

int main(int argc, char *argv[]) {
    
	char *cipher_buffer;
	char *key_buffer;
    int c_size = 36;
    char plaintext_buffer[c_size];

    cipher_buffer = "GYEOSZMHHHIYLNQRQZDUOYALOPEUAGWIHLLF";
    key_buffer = "ORAPBVJIBUVGHOLGIVMVOFB GMSMV DJQTYR";

    int i;
    for(i = 0; i < c_size; i++) {
        // plaintext_buffer[i] = decrypt(cipher_buffer[i], key_buffer[i]);
        printf("%c", decrypt(cipher_buffer[i], key_buffer[i]));
    }
    printf("\n");
    fflush(stdout);

    // printf("Plaintext: %s\n", plaintext_buffer);
    // fflush(stdout);

    return 0;
}