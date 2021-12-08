#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sstream>

#define ADDRESS 0b10
#define REMOTE_ADDRESS 0b01
#define GCC_OUT_FILE "gcc_out.temp"

int cd(int socket, packet_t *p, unsigned char *buffer) {

    std::vector<packet_t> v1, v2;
    char *current_dir = (char *)malloc(sizeof(char)*STRING_BUFFERSIZE);

    current_dir = getcwd(current_dir, STRING_BUFFERSIZE);
    std::string dir(current_dir);

    printf("on cd\n");

    v1.push_back(*p);
    if (!last_packet(p)) {
        v2 = receive_until_termination(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    dir += '/';
    printf("packet to string: %s\n", packet_to_string(v1).c_str());
    dir += packet_to_string(v1);

    printf("trying to cd to %s\n", dir.c_str());

    if (chdir(dir.c_str()) == 0) {
        send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
        return 0;
    }
    else if (errno == EACCES)
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 1);
    else if ((errno == ENOENT) || (errno == ENOTDIR))
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 2);
    else
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);
        

    return -1;

} 

int ls(int socket, packet_t *p, unsigned char *buffer, unsigned char *copy_buffer) {

    std::string ls;
    std::vector<packet_t> v1, v2;
    DIR *dir;
    struct dirent *ent;
    char *current_dir = (char *)malloc(sizeof(char)*STRING_BUFFERSIZE);
    std::stringstream ss;
    std::string s;

    printf("on ls\n");

    current_dir = getcwd(current_dir, STRING_BUFFERSIZE);

    v1.push_back(*p);

    if (!last_packet(p)) {
        send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
        v2 = receive_all_no_response(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    ss << current_dir << "/" << packet_to_string(v1).c_str();
    s = ss.str();
    
    printf("trying to ls dir %s\n", s.c_str());

    ls = "";
    if((dir = opendir(s.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            ls += ent->d_name;
            ls += " ";
        }
        closedir(dir);

        printf("ls output: %s\n", ls.c_str());

        memcpy(buffer, ls.c_str(), ls.size()+1);
        send_any_size(socket, buffer, copy_buffer, ls.size()+1, 0b1011, REMOTE_ADDRESS, ADDRESS);
    }
    else if (errno == EACCES)
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 1);
    else if ((errno == ENOENT) || (errno == ENOTDIR))
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 2);
    else
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);

    return 0;
} 

int ver(int socket, packet_t *p, unsigned char *buffer, unsigned char *copy_buffer) {
    FILE *fp=NULL;
    std::vector<packet_t> v1, v2;
    std::string out, filename;
    char *s1, *s2;
    DIR *dir;
    bool exists = false;

    printf("on ver\n");

    s1 = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));
    s2 = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));

    v1.push_back(*p);
    if (!last_packet(p)) {
        v2 = receive_until_termination(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    filename = packet_to_string(v1);

    dir = opendir(filename.c_str());
    if (dir)
        closedir(dir);
    if(errno == ENOTDIR)
        exists = true;

    if (!exists) {
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
        return -1;
    }

    fp = fopen(filename.c_str(), "r");

    if (fp != NULL) {
        int i=0;
        out = "";
        while(fscanf(fp, "%[^\n]\n", s2) > 0) {

            i++;
            std::sprintf(s1, "%d ", i);
            out = (((out+s1)+s2)+"\n");
        }
        fclose(fp);


        
        memcpy(buffer, out.c_str(), out.size()+1);
        send_any_size(socket, buffer, copy_buffer, out.size()+1, 0b1100, REMOTE_ADDRESS, ADDRESS); 
        send_any_size(socket, buffer, copy_buffer, 0, 0b1101, REMOTE_ADDRESS, ADDRESS);
        return 0;
    }
    else if (errno == EACCES)
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 1);
    else if ((errno == EISDIR) || (errno == ENOENT) || (errno == ENOTDIR))
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
    else
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);

   
    if (fp != NULL)
        fclose(fp);

    return -1;

} 

int linha(int socket, packet_t *p, unsigned char *buffer, unsigned char *copy_buffer) {
    FILE *fp;
    std::vector<packet_t> v1, v2;
    std::string out, filename;
    char *s1, *s2;
    int line=0;
    int *p_int;
    DIR *dir;
    bool exists = false;

    printf("on linha\n");

    s1 = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));
    s2 = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));

    v1.push_back(*p);
    if (!last_packet(p)) {
        v2 = receive_until_termination(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    filename = packet_to_string(v1);

    dir = opendir(filename.c_str());
    if (dir)
        closedir(dir);
    if(errno == ENOTDIR)
        exists = true;

    if (!exists) {
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
        return -1;
    }

    fp = fopen(filename.c_str(), "r");

    if (fp != NULL) {

        send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
        v1 = receive_until_termination(socket, buffer, ADDRESS);

        if (v1.at(0).tam == sizeof(int)) {
            p_int = (int *)v1.at(0).dados;
            line = p_int[0];
        }

        int i=0;
        out = "";
        while(fscanf(fp, "%[^\n]\n", s2) > 0) {

            i++;
//            printf("i = %d", i);
            if ((line == i) || (line == 0)) {
                std::sprintf(s1, "%d ", i);
                out = (((out+s1)+s2)+"\n");
            }
        }
        fclose(fp);

//        printf("ver output (%d):\n%s", out.size()+1, out.c_str());
        
        memcpy(buffer, out.c_str(), out.size()+1);
        send_any_size(socket, buffer, copy_buffer, out.size()+1, 0b1100, REMOTE_ADDRESS, ADDRESS); 
        send_any_size(socket, buffer, copy_buffer, 0, 0b1101, REMOTE_ADDRESS, ADDRESS);
        return 0;
    }
    else if (errno == EACCES)
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 1);
    else if ((errno == EISDIR) || (errno == ENOENT) || (errno == ENOTDIR))
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
    else
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);

    fclose(fp);

    return -1;



} 

int linhas(int socket, packet_t *p, unsigned char *buffer, unsigned char *copy_buffer) {
    FILE *fp;
    std::vector<packet_t> v1, v2;
    std::string out, filename;
    char *s1, *s2;
    int upper, lower;
    int *p_int;
    DIR *dir;
    bool exists = false;

printf("on linhas\n");

    upper = 0;
    lower = 0;

    s1 = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));
    s2 = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));

    v1.push_back(*p);
    if (!last_packet(p)) {
        v2 = receive_until_termination(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    filename = packet_to_string(v1);

    dir = opendir(filename.c_str());
    if (dir)
        closedir(dir);
    if(errno == ENOTDIR)
        exists = true;

    if (!exists) {
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
        return -1;
    }

    fp = fopen(filename.c_str(), "r");

    if (fp != NULL) {

        send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
        v1 = receive_until_termination(socket, buffer, ADDRESS);

        if (v1.at(0).tam == 2*sizeof(int)) {
            p_int = (int *)v1.at(0).dados;
            lower = p_int[0];
            upper = p_int[1];
        }
        else if (v1.at(0).tam == sizeof(int)) {
            p_int = (int *)v1.at(0).dados;
            lower = p_int[0];
            upper = -1;
        }
        else {
            lower = upper = -1;
        }

        int i=0;
        out = "";
        while(fscanf(fp, "%[^\n]\n", s2) > 0) {

            i++;
//            printf("i = %d", i);
            if (
                ((i >= lower) && (i <= upper)) ||
                ((i >= lower) && (upper == -1)) ||
                ((lower == -1) && (upper == -1))
                ) {
                std::sprintf(s1, "%d ", i);
                out = (((out+s1)+s2)+"\n");
            }
        }
        fclose(fp);

//        printf("ver output (%d):\n%s", out.size()+1, out.c_str());
        
        memcpy(buffer, out.c_str(), out.size()+1);
        send_any_size(socket, buffer, copy_buffer, out.size()+1, 0b1100, REMOTE_ADDRESS, ADDRESS); 
        send_any_size(socket, buffer, copy_buffer, 0, 0b1101, REMOTE_ADDRESS, ADDRESS);
        return 0;
    }
    else if (errno == EACCES)
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 1);
    else if ((errno == EISDIR) || (errno == ENOENT) || (errno == ENOTDIR))
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
    else
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);

    fclose(fp);

    return -1;

} 

int edit(int socket, packet_t *p, unsigned char *buffer, unsigned char *copy_buffer) {
    FILE *fp;
    std::vector<packet_t> v1, v2;
    std::string out, filename, edit_line;
    std::vector<std::string> file_text;
    packet_t p_aux;
    char *s;
    int line, i;
    DIR *dir;

printf("on edit\n");

    s = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));

    v1.push_back(*p);
    if (!last_packet(p)) {
        v2 = receive_until_termination(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    filename = packet_to_string(v1);

    dir = opendir(filename.c_str());
    if (dir) {
        closedir(dir);
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);
        return -1;
    }

    fp = fopen(filename.c_str(), "r");

    if (fp != NULL) {

        send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);

        v1 = receive_all_no_response(socket, buffer, ADDRESS);
        p_aux = v1.at(0);

        if (p_aux.tam == sizeof(int)) {
            line = *((int *)p_aux.dados);
        }

        while(fscanf(fp, "%[^\n]\n", s) > 0)
            file_text.push_back(s);
        fclose(fp);        

        if (line > (int)file_text.size()+1) {
            send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 4);
            return -1;
        }

        send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);

        edit_line = packet_to_string(receive_until_termination(socket, buffer, ADDRESS)); 

        file_text.at(line-1) = edit_line;

        fp = fopen(filename.c_str(), "w");

        for(auto i: file_text) {

            fprintf(fp, "%s\n", i.c_str());

        }

        fclose(fp);        

        return 0;
    }
    else if (errno == EACCES)
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 1);
    else if ((errno == EISDIR) || (errno == ENOENT) || (errno == ENOTDIR))
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 3);
    else
        send_error(socket, buffer, REMOTE_ADDRESS, ADDRESS, 5);

    fclose(fp);

    return -1;

} 

int compilar(int socket, packet_t *p, unsigned char *buffer, unsigned char *copy_buffer) {
    FILE *fp;
    std::vector<packet_t> v1, v2;
    std::string out, line;
    std::stringstream ss;
    char *s;
    DIR *dir;
    bool exists = false;

    printf("i'm in compilar!\n");

    s = (char *)malloc(STRING_BUFFERSIZE*sizeof(char));

    send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
    v1.push_back(*p);
    if (!last_packet(p)) {
        v2 = receive_until_termination(socket, buffer, ADDRESS);
        v1.insert(v1.end(), v2.begin(), v2.end());
    }

    ss << "gcc " << packet_to_string(v1).c_str() << " 2> " << GCC_OUT_FILE;
    line = ss.str();

    printf("gcc line: %s\n", line.c_str());

    system(line.c_str());

    printf("opening file\n");
    dir = opendir(GCC_OUT_FILE);
    if (dir)
        closedir(dir);
    if(errno == ENOTDIR)
        exists = true;

    if (!exists) {
        send_any_size(socket, buffer, copy_buffer, 0, 0b1101, REMOTE_ADDRESS, ADDRESS);
        return 0;
    }

    fp = fopen(GCC_OUT_FILE, "r");

    out = "";
    while(fscanf(fp, "%[^\n]\n", s) > 0) {
        out = (out+s)+"\n";
    }
   
    fclose(fp); 

    ss << "rm " << GCC_OUT_FILE;
    line = ss.str();

    system(line.c_str());
        
    memcpy(buffer, out.c_str(), out.size()+1);
    send_any_size(socket, buffer, copy_buffer, out.size()+1, 0b1100, REMOTE_ADDRESS, ADDRESS); 
    send_any_size(socket, buffer, copy_buffer, 0, 0b1101, REMOTE_ADDRESS, ADDRESS);
    return 0;

} 


int main () {

    int socket;
    unsigned char *buffer, *copy_buffer;
    packet_t *p;
    bool end = false;

    buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    copy_buffer = (unsigned char *)malloc(BUFFERSIZE*sizeof(unsigned char));
    socket = ConexaoRawSocket("lo");
    //std::cout << "socket " << socket << " initialized" << std::endl;

    while (!end) {
        p = receive_and_respond(socket, buffer, ADDRESS);
        if (p != NULL) {
            if (p->tipo == 0b0000)
                cd(socket, p, buffer);
                
            if (p->tipo == 0b0001)
                ls(socket, p, buffer, copy_buffer);

            if (p->tipo == 0b0010)
                ver(socket, p, buffer, copy_buffer);

            if (p->tipo == 0b0011)
                linha(socket, p, buffer, copy_buffer);

            if (p->tipo == 0b0100)
                linhas(socket, p, buffer, copy_buffer);

            if (p->tipo == 0b0101)
                edit(socket, p, buffer, copy_buffer);

            if (p->tipo == 0b0110)
                compilar(socket, p, buffer, copy_buffer);

            if (p->tipo == 0b1101) {
                end = true;
                send_ACK(socket, buffer, REMOTE_ADDRESS, ADDRESS, 0);
            }
        }

    }
}
