#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define ADDRESS 0b01

void cd(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on cd\n");
}
void lcd(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on lcd\n");
}
void ls(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {

    printf("on ls\n");
}
void lls(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on lls\n");
}
void ver(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on ver\n");
}
void linha(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on linha\n");
}
void linhas(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on linhas\n");
}
void edit(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on edit\n");
}
void compilar(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on compilar\n");
}

int main () {

    int socket;
    unsigned char *buffer, *copy_buffer;
    std::string s;
    std::vector<std::string> input;
    bool end;

    
    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    copy_buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");

    end = false;

    while (!end) {
        std::getline(std::cin, s);
        input = separate_string(s, ' ');

        if (input.size() >= 1)
            if (input.at(0) == "cd") {
                cd(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "lcd") {
                lcd(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "ls") {
                ls(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "lls") {
                lls(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "ver") {
                ver(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "linha") {
                linha(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "linhas") {
                linhas(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "edit") {
                edit(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "compilar") {
                compilar(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "exit") {
                end = true;
            }
            else {
                printf("unrecognized command, try again\n");
            }       

/*
        if (s.size() > 0) std::cout << "received " << s << " from input" << std::endl;
        memcpy(buffer, s.c_str(), s.size()+1);
        send_any_size(socket, buffer, copy_buffer, s.size()+1, 0b0111, 0b10, ADDRESS);
*/
    }
}
