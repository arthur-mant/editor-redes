#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main () {

    int socket;
    unsigned char *buffer;
    
    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");
    std::cout << "socket " << socket << " initialized" << std::endl;

    while (1) {
        for (auto i: receive_from_socket(socket, buffer)) {
            printf("received %d bytes from %d, type = %d, sequence number = %d\n", i.tam, i.e_origem, i.tipo, i.sequencia);
            for (int j=0; j<i.tam; j++)
                printf("%c ", i.dados[j]);
            printf("\n");
        }
    }
}
