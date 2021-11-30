typedef struct packet_t {

    unsigned int e_destino;
    unsigned int e_origem;
    unsigned int tam;
    unsigned int sequencia;
    unsigned int tipo;
    unsigned int *dados;

} packet_t;
