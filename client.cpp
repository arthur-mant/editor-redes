#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define ADDRESS 0b01
#define REMOTE_ADDRESS 0b10

void print_error(int error) {

    switch (error) {
        case 1:
        printf("Falta de permissão\n");
        break;
        case 2:
        printf("Diretório inexistente\n");
        break;
        case 3:
        printf("Arquivo inexistente\n");
        break;
        case 4:
        printf("Linha inexistente\n");
        break;
        case 5:
        printf("Erro não definido\n");
        break;
    }
}

int cd(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {

    std::vector<packet_t> response;

    if (v.size() < 2) {
        printf("please specify a directory\n");
        return -1;
    }

    memcpy(buffer, v.at(1).c_str(), v.at(1).size()+1);
    response = send_any_size(socket, buffer, copy_buffer, v.at(1).size()+1, 0b0000, REMOTE_ADDRESS, ADDRESS);

    for (auto i: response) {
        if (i.tipo == 0b1111)
            print_error(i.dados[0]);
        else if(i.tipo != 0b1000)
            printf("got a type %d response (?)\n", i.tipo);
    }

    return 0;

}

void lcd(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    printf("on lcd\n");
}

void ls(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {

    std::vector<packet_t> response, aux;

    response = send_any_size(socket, buffer, copy_buffer, 0, 0b0001, REMOTE_ADDRESS, ADDRESS);

    send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
    aux = receive_until_termination(socket, buffer, ADDRESS);


    response.insert(response.end(), aux.begin(), aux.end());

    for (auto i: response) {
        if (i.tipo == 0b1111)
            print_error(i.dados[0]);
        else if(i.tipo == 0b1011) {
            for (int j=0; j<i.tam; j++)
                printf("%c", i.dados[j]);
        }       
        else if(i.tipo != 0b1000)
            printf("got a type %d response (?)\n", i.tipo);
    }
    printf("\n");

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
