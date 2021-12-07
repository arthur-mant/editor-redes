#include <vector>
#include <string>
#include <string.h>

typedef struct packet_t {

    unsigned int e_destino;
    unsigned int e_origem;
    unsigned int tam;
    unsigned int sequencia;
    unsigned int tipo;
    unsigned char *dados;
    unsigned int parity;

} packet_t;

#define BUFFERSIZE 65536
#define TIMEOUT 1000        //in ms
#define MAX_RETRIES 5
#define STRING_BUFFERSIZE 1000

int ConexaoRawSocket(char *device);
std::vector<packet_t> send_any_size(int socket, unsigned char *buffer, unsigned char *copy_buffer, int tam, int tipo, int destino, int origem);
packet_t *receive_and_respond(int socket, unsigned char *buffer, int endereco);
std::vector<packet_t> receive_all_no_response(int socket, unsigned char *buffer, int endereco);
std::vector<packet_t> receive_until_termination(int socket, unsigned char *buffer, int endereco);
std::vector<std::string>separate_string(std::string, char);
int send_ACK(int socket, unsigned char *buffer, int destino, int origem, int sequencia);
int send_error(int socket, unsigned char *buffer, int destino, int origem, int error);
bool last_packet(packet_t *p);
std::string packet_to_string(std::vector<packet_t> v);

