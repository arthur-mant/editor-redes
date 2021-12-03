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
#define TIMEOUT 5000        //in ms
#define MAX_RETRIES 5

int ConexaoRawSocket(char *device);
//unsigned char *empacota(unsigned char *buffer, int tamanho, int tipo, int destino, int origem, int sequencia);
int send_to_socket(int socket, unsigned char *buffer, int tam, int tipo, int destino, int origem);
//packet_t *desempacota(unsigned char *data);
std::vector<packet_t> receive_from_socket(int socket, unsigned char *buffer);

