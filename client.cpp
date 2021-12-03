#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>

#define ADDRESS 0b01

int main () {

    int socket;
    unsigned char *buffer, *copy_buffer;
    std::string s;
    
    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    copy_buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");

    std::cout << "socket " << socket << " initialized" << std::endl;

    while (1) {
        std::cin >> s;
        if (s.size() > 0) std::cout << "received " << s << " from input" << std::endl;
        memcpy(buffer, s.c_str(), s.size()+1);
        send_any_size(socket, buffer, copy_buffer, s.size()+1, 0b0111, 0b10, ADDRESS);
    }
}
