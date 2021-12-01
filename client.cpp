#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>

int main () {

    int socket;
    unsigned char *buffer;
    std::string s;
    
    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");

    std::cout << "socket " << socket << " initialized" << std::endl;

    while (1) {
        std::cin >> s;
        if (s.size() > 0) std::cout << "received " << s << " from input" << std::endl;
        memcpy(buffer, s.c_str(), s.size()+1);
        send_to_socket(socket, buffer, s.size()+1, 0b0111, 0b10, 0b01);
    }
}
