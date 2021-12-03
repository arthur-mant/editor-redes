#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define ADDRESS 0b10

int main () {

    int socket;
    unsigned char *buffer;
    std::vector<packet_t> v;    

    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");
    std::cout << "socket " << socket << " initialized" << std::endl;

    while (1) {
        v = receive_until_termination(socket, buffer, ADDRESS);
        for(auto p: v) {
            printf("received %d bytes from %d, type = %d, sequence number = %d\n", p.tam, p.e_origem, p.tipo, p.sequencia);
            for (int j=0; j<p.tam; j++)
                printf("%c ", p.dados[j]);
            printf("\n");
        }        
    }
}
