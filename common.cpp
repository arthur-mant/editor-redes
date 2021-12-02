#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <netinet/in.h>

#ifndef COMMON
#define COMMON
#include "common.h"
#endif

//função do albini
//utilizar Loop Back ("lo") como device
int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	


  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
}

unsigned char *empacota(unsigned char *buffer, int tamanho, int tipo, int destino, int origem, int sequencia) {

    //[inicio, destino origem tamanho, sequencia tipo, dados, paridade]

    unsigned char *packet;
    unsigned char inicio, dest_c, orig_c, tam_c, seq_c, tipo_c, paridade, tmp;

    //packet = (unsigned char *)malloc(8+2+2+4+4+4+tamanho+8);
    packet = (unsigned char *)malloc(19);

    inicio = 0b01111110;
    packet[0] = inicio;

    dest_c = (unsigned char) destino;
    orig_c = (unsigned char) origem;
    tam_c = (unsigned char) tamanho;
    packet[1] = (dest_c << (2+4)) | (orig_c << 4) | (tam_c);

    printf("seq: %#04x, tipo: %#04x\n", sequencia, tipo);
    seq_c = (unsigned char) sequencia;
    tipo_c = (unsigned char) tipo;
    packet[2] = (seq_c << 4) | (tipo_c);

    tmp = tam_c ^ seq_c ^ tipo_c;

    for(int i=0; i<tamanho; i++) {
        packet[3+i] = buffer[i];
        tmp = tmp ^ buffer[i];
    }
    
    packet[3+tamanho] = tmp;

    for (int i=3+tamanho+1; i<19; i++)
        packet[i] = 0;


    //packet_len = 4+tamanho
    return packet;
}

int send_to_socket(int socket, unsigned char *buffer, int tam, int tipo, int destino, int origem) {

    int full_p, tam_leftover_p, index;
    unsigned char *aux;
/*
    printf("sending following byte string to socket %d\n[", socket);
    for (int i=0; i<tam; i++)
        printf("%#04x, ", *(buffer+i));
    printf("]\n");
*/
    full_p = tam/15;
    tam_leftover_p = tam % 15;

    index = 0;
    for (int i=0; i<full_p; i++) {

        aux = empacota(buffer + 15*i, 15, tipo, destino, origem, index),
        printf("sending following byte string to socket %d\n[", socket);
        for (int i=0; i<15+4; i++)
            printf("%#04x, ", *(aux+i));
        printf("]\n");

        send(
            socket,
            (void *)aux,
            //(void *)empacota(buffer + 15*i, 15, tipo, destino, origem, index),
            15+4, 0
        );

        index = (index+1) % 16;
        
    }

    if ((tam_leftover_p > 0) || (full_p == 0)) {
        aux = empacota(buffer + 15*full_p, tam_leftover_p, tipo, destino, origem, index),
        printf("sending following byte string to socket %d\n[", socket);
        for (int i=0; i<tam_leftover_p+4; i++)
            printf("%#04x, ", *(aux+i));
        printf("]\n");

        send(
            socket,
            (void *)aux,
            //(void *)empacota(buffer + 15*full_p, tam_leftover_p, tipo, destino, origem, (index+1) % 16),
            //tam_leftover_p+4, 0
            19, 0
        );


    }
}

packet_t *desempacota(unsigned char *data) {

    packet_t *p = (packet_t *)malloc(sizeof(packet_t));
    unsigned char parity;

    p->e_destino = data[1] >> 6;
    p->e_origem = (data[1] & 0b00110000) >> 4;
    p->tam = data[1] & 0b00001111;
    p->sequencia = data[2] >> 4;
    p->tipo = data[2] & 0b00001111;

    parity = p->tam ^ p->sequencia ^ p->tipo;

    if (p->tam > 0) {
        p->dados = (unsigned char *)malloc(p->tam*sizeof(unsigned char));
        for (int i=0; i<p->tam; i++) {
            p->dados[i] = data[3+i];
            parity = parity ^ data[3+i];
        }
    }

    if (parity != data[3+p->tam]) {
        printf("ERROR: parity byte wrong\n");
        return NULL;
    }
    return p;
    
}

std::vector<packet_t> receive_from_socket(int socket, unsigned char *buffer) {

    int buflen, saddr_len;
    packet_t *tmp;
    std::vector<packet_t> v;
    struct sockaddr_ll saddr;

    saddr_len = sizeof (struct sockaddr_ll);

//    buflen = recv(socket, buffer, BUFFERSIZE, 0);
    buflen = recvfrom(socket, buffer, BUFFERSIZE, 0, (struct sockaddr *)&saddr, (socklen_t *)&saddr_len);

    if (buflen < 0) {
        printf("error reading recvfrom function\n");
        return {};
    }
    //evita receber duplicatas
    if (saddr.sll_pkttype == PACKET_OUTGOING) {
        //printf("ignoring outgoing packet\n");
        return {};
    }

    for(int i=0; i<buflen; i++)
        if (*(buffer+i) == 0b01111110) {
            tmp = desempacota(buffer+i);
            if (tmp != NULL)
                v.push_back(*tmp);
        }

    return v;

}
