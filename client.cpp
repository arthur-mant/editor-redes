#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <algorithm>

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

int lcd(std::vector<std::string> v) {

    char *current_dir = (char *)malloc(sizeof(char)*STRING_BUFFERSIZE);

    current_dir = getcwd(current_dir, STRING_BUFFERSIZE);
    std::string dir(current_dir);

    if (v.size() < 2) {
        printf("please specify a directory\n");
        return -1;
    }

    dir += '/';
    dir += v.at(1);

    printf("trying to cd to %s\n", dir.c_str());

    if (chdir(dir.c_str()) == 0) {
        return 0;
    }
    else if (errno == EACCES)
        print_error(1);
    else if ((errno == ENOENT) || (errno == ENOTDIR))
        print_error(2);
    else
        print_error(5);
    return -1;
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
int lls(std::vector<std::string> v) {
    std::string ls;
    DIR *dir;
    struct dirent *ent;
    char *current_dir = (char *)malloc(sizeof(char)*STRING_BUFFERSIZE);

    current_dir = getcwd(current_dir, STRING_BUFFERSIZE);
    std::string s(current_dir);

/*
    if (v.size() >= 2) {
        s += '/';
        s += v.at(1);
    }
*/
    
    printf("trying to ls dir %s\n", s.c_str());

    ls = "";
    if((dir = opendir(s.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            ls += ent->d_name;
            ls += " ";
        }
        closedir(dir);

        printf("%s\n", ls.c_str());
        return 0;
    }
    else if (errno == EACCES)
        print_error(1);
    else if ((errno == ENOENT) || (errno == ENOTDIR))
        print_error(2);
    else
        print_error(5);

    return -1;
    
}
int ver(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    std::vector<packet_t> response, aux;

    if (v.size() < 2) {
        printf("please specify a file\n");
        return -1;
    }

    memcpy(buffer, v.at(1).c_str(), v.at(1).size()+1);
    response = send_any_size(socket, buffer, copy_buffer, v.at(1).size()+1, 0b0010, REMOTE_ADDRESS, ADDRESS);

    if (response.at(0).tipo == 0b1111)
            print_error(response.at(0).dados[0]);
        
    send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
    aux = receive_until_termination(socket, buffer, ADDRESS);


    response.insert(response.end(), aux.begin(), aux.end());


    for (auto i: response) {
        if (i.tipo == 0b1100) {
            for(int j=0; j<i.tam; j++)
                printf("%c", i.dados[j]);

        }
        else if (i.tipo == 0b1111)
            print_error(i.dados[0]);
        else if((i.tipo != 0b1000) && (i.tipo != 0b1101))
            printf("got a type %d response (?)\n", i.tipo);
    }

    return 0;
}
int linha(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    std::vector<packet_t> response, aux;
    int line=0;

    if (v.size() < 2) {
        printf("please specify a file\n");
        return -1;
    }

    memcpy(buffer, v.at(1).c_str(), v.at(1).size()+1);
    response = send_any_size(socket, buffer, copy_buffer, v.at(1).size()+1, 0b0011, REMOTE_ADDRESS, ADDRESS);
    for (auto i: response) {
        if (i.tipo == 0b1111) {
            print_error(i.dados[0]);
            return -1;
        }
        else if(i.tipo != 0b1000) {
            printf("got a type %d response (?)\n", i.tipo);
            return -1;
        }
    }

    if ((v.size() >= 3) && (std::stoi(v.at(2)) > 0))
        line = std::stoi(v.at(2));

    memcpy(buffer, &line, sizeof(int));
    response = send_any_size(socket, buffer, copy_buffer, sizeof(int), 0b1010, REMOTE_ADDRESS, ADDRESS);

    aux = receive_until_termination(socket, buffer, ADDRESS);

    response.insert(response.end(), aux.begin(), aux.end());


    for (auto i: response) {
        if (i.tipo == 0b1100) {
            for(int j=0; j<i.tam; j++)
                printf("%c", i.dados[j]);

        }
        else if (i.tipo == 0b1111)
            print_error(i.dados[0]);
        else if((i.tipo != 0b1000) && (i.tipo != 0b1101))
            printf("got a type %d response (?)\n", i.tipo);
    }

    return 0;
}
int linhas(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    std::vector<packet_t> response, aux;
    int line[10];

    if (v.size() < 2) {
        printf("please specify a file\n");
        return -1;
    }

    memcpy(buffer, v.at(1).c_str(), v.at(1).size()+1);
    response = send_any_size(socket, buffer, copy_buffer, v.at(1).size()+1, 0b0100, REMOTE_ADDRESS, ADDRESS);
    for (auto i: response) {
        if (i.tipo == 0b1111) {
            print_error(i.dados[0]);
            return -1;
        }
        else if(i.tipo != 0b1000) {
            printf("got a type %d response (?)\n", i.tipo);
            return -1;
        }
    }

    if ((v.size() >= 3) && (std::stoi(v.at(2)) > 0))
        line[0] = std::stoi(v.at(2));
    if ((v.size() >= 4) && (std::stoi(v.at(3)) > 0))
        line[1] = std::stoi(v.at(3));

    memcpy(buffer, line, std::max(0,(int)v.size()-2)*sizeof(int));
    response = send_any_size(socket, buffer, copy_buffer, std::max(0, (int)v.size()-2)*sizeof(int), 0b1010, REMOTE_ADDRESS, ADDRESS);

    aux = receive_until_termination(socket, buffer, ADDRESS);

    response.insert(response.end(), aux.begin(), aux.end());

    for (auto i: response) {
        if (i.tipo == 0b1100) {
            for(int j=0; j<i.tam; j++)
                printf("%c", i.dados[j]);

        }
        else if (i.tipo == 0b1111)
            print_error(i.dados[0]);
        else if((i.tipo != 0b1000) && (i.tipo != 0b1101))
            printf("got a type %d response (?)\n", i.tipo);
    }

    return 0;
}
int edit(std::vector<std::string> v, int socket, unsigned char *buffer, unsigned char *copy_buffer) {
    std::vector<packet_t> response, aux;
    std::string s;
    bool writing_line;
    int line = -1;
    printf("on edit\n");

    if (v.size() < 4) {
        printf("please specify a file, line number and new line\n");
        return -1;
    }

    memcpy(buffer, v.at(1).c_str(), v.at(1).size()+1);
    response = send_any_size(socket, buffer, copy_buffer, v.at(1).size()+1, 0b0101, REMOTE_ADDRESS, ADDRESS);
    for (auto i: response) {
        if (i.tipo == 0b1111) {
            print_error(i.dados[0]);
            return -1;
        }
        else if(i.tipo != 0b1000) {
            printf("got a type %d response (?)\n", i.tipo);
            return -1;
        }
    }

    line = std::stoi(v.at(2));

    memcpy(buffer, &line, sizeof(int));
    response = send_any_size(socket, buffer, copy_buffer, sizeof(int), 0b1010, REMOTE_ADDRESS, ADDRESS);
    for (auto i: response) {
        if (i.tipo == 0b1111) {
            print_error(i.dados[0]);
            return -1;
        }
        else if(i.tipo != 0b1000) {
            printf("got a type %d response (?)\n", i.tipo);
            return -1;
        }
    }

    s = "";
    writing_line = false;
    for (auto i: v) {
        if (i.at(0) == '"')
            writing_line = true;
        if (writing_line) {
            if (i.at(0) == '"')
                s += i.substr(1, i.size());
            else if (i.at(i.size()-1) == '"')
                s += i.substr(0, i.size()-1);
            else 
                s+=i;
            s+=" ";
        }
        if (i.at(i.size()-1) == '"')
            writing_line = false;
    }

    printf("line to write: %s\n", s.c_str());

    memcpy(buffer, s.c_str(), s.size()+1);
    response = send_any_size(socket, buffer, copy_buffer, s.size()+1, 0b1100, REMOTE_ADDRESS, ADDRESS);

    for (auto i: response) {
        if (i.tipo == 0b1111) {
            print_error(i.dados[0]);
            return -1;
        }
        else if(i.tipo != 0b1000) {
            printf("got a type %d response (?)\n", i.tipo);
            return -1;
        }
    }

    return 0;
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
                lcd(input);
            }
            else if (input.at(0) == "ls") {
                ls(input, socket, buffer, copy_buffer);
            }
            else if (input.at(0) == "lls") {
                lls(input);
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
