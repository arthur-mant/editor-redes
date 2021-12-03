#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define ADDRESS 0b10

int main () {

    int socket;
    unsigned char *buffer;
    packet_t *p;    

    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");
    std::cout << "socket " << socket << " initialized" << std::endl;

    while (1) {
        p = receive_and_respond(socket, buffer, ADDRESS);
        if (p != NULL) {
            printf("received %d bytes from %d, type = %d, sequence number = %d\n", p->tam, p->e_origem, p->tipo, p->sequencia);
            for (int j=0; j<p->tam; j++)
                printf("%c ", p->dados[j]);
            printf("\n");
        }        
    }
}
