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

int ConexaoRawSocket(char *device);
int send_any_size(int socket, unsigned char *buffer, unsigned char *copy_buffer, int tam, int tipo, int destino, int origem);
packet_t *receive_and_respond(int socket, unsigned char *buffer, int endereco);
