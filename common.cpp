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
#include <chrono>

#ifndef COMMON
#define COMMON
#include "common.h"
#endif

int receive_from_socket(int socket, unsigned char *buffer, packet_t **out, int endereco);

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

    packet = (unsigned char *)malloc(20*sizeof(unsigned char));

    inicio = 0b01111110;
    packet[0] = inicio;

    dest_c = (unsigned char) destino;
    orig_c = (unsigned char) origem;
    tam_c = (unsigned char) tamanho;
    packet[1] = (dest_c << (2+4)) | (orig_c << 4) | (tam_c);

    //printf("seq: %#04x, tipo: %#04x\n", sequencia, tipo);
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

int send_to_socket(int socket, unsigned char *buffer, int tam, int tipo, int destino, int origem, int sequencia) {

    return send(
        socket,
        (void *)empacota(buffer, tam, tipo, destino, origem, sequencia),
        15+4, 0
    );
}

int send_and_wait(int socket, unsigned char *buffer, int tam, int tipo, int destino, int origem, int sequencia) {

    bool ack;
    packet_t *p;
    int retry = 0;

    do {
        send_to_socket(socket, buffer, tam, tipo, destino, origem, sequencia);

        auto start = std::chrono::high_resolution_clock::now();
        ack = false;
        do {
            if (receive_from_socket(socket, buffer, &p, origem) == 0) {
                if (p->tipo == 0b1000)
                    ack = true;
                if (p->tipo == 0b1001)
                    send_to_socket(socket, buffer, tam, tipo, destino, origem, sequencia);
            }
        } while ((!ack) &&
            (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < TIMEOUT));
        retry++;
        if (!ack) printf("TIMEOUT, retrying...\n");
    } while ((!ack) && (retry < MAX_RETRIES));

    if (!ack) {
        printf("ERROR: Max tries reached, something's wrong...\n");
        return -1;
    }
    return 0;
}

int send_any_size(int socket, unsigned char *buffer, unsigned char *copy_buffer, int tam, int tipo, int destino, int origem) {

    int index = 0;

    do {
        memcpy(copy_buffer, buffer, std::min(tam, 15));

        if (send_and_wait(socket, copy_buffer, std::min(tam, 15), tipo, destino, origem, index) == -1) {
            printf("PANIC!!!\n");
            return -1;
        }
        tam = tam - 15;
        buffer = buffer + 15;
        index = (index + 1) % 16;
    } while(tam > 0);

    return 0;

}

packet_t *desempacota(unsigned char *data) {

    packet_t *p = (packet_t *)malloc(sizeof(packet_t));
    unsigned char parity;

    p->e_destino = data[1] >> 6;
    p->e_origem = (data[1] & 0b00110000) >> 4;
    p->tam = data[1] & 0b00001111;
    p->sequencia = data[2] >> 4;
    p->tipo = data[2] & 0b00001111;



    if (p->tam > 0) {
        p->dados = (unsigned char *)malloc(p->tam*sizeof(unsigned char));
        for (int i=0; i<p->tam; i++)
            p->dados[i] = data[3+i];
    }
    p->parity = data[3+p->tam];

    return p;
    
}

int receive_from_socket(int socket, unsigned char *buffer, packet_t **out, int endereco) {

    int buflen, saddr_len;
    packet_t *p;
    struct sockaddr_ll saddr;
    unsigned int parity;

    saddr_len = sizeof (struct sockaddr_ll);

//    buflen = recv(socket, buffer, BUFFERSIZE, 0);
    buflen = recvfrom(socket, buffer, BUFFERSIZE, 0, (struct sockaddr *)&saddr, (socklen_t *)&saddr_len);

    if (buflen < 0) {
        printf("error reading recvfrom function\n");
        return -3;
    }
    //evita receber duplicatas
    if (saddr.sll_pkttype == PACKET_OUTGOING) {
        //printf("ignoring outgoing packet\n");
        return -3;
    }

    for(int i=0; i<buflen; i++)
        if (*(buffer+i) == 0b01111110) {
            p = desempacota(buffer+i);

            parity = p->tam ^ p->sequencia ^ p->tipo;
            for (int j=0; j<p->tam; j++)
                parity = parity ^ p->dados[j];
            if ((parity == p->parity) && (p->e_destino == endereco)) {
                *out = p;
                return 0;
            }
            else if (p->e_destino == endereco){
                *out = p;
                return -1;
            }
        }
    return -2;
}



packet_t *receive_and_respond(int socket, unsigned char *buffer, int endereco) {

    int aux, retry;
    packet_t *p;

    aux = receive_from_socket(socket, buffer, &p, endereco);

    retry = 0;

    if (aux == 0) {
        //send ack
        send_to_socket(socket, buffer, 0, 0b1000, p->e_origem, p->e_destino, 0);
    }
    else if (aux == -1) {
        //send nack
        send_to_socket(socket, buffer, 0, 0b1001, p->e_origem, p->e_destino, 0);
        //wait for reponse
        do {
            auto start = std::chrono::high_resolution_clock::now();
            do {
                aux = receive_from_socket(socket, buffer, &p, endereco);
            } while ((aux != 0) &&
                (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < TIMEOUT));
            retry++;
            if (aux != 0) printf("TIMEOUT, retrying...\n");
        } while ((aux != 0) && (retry < MAX_RETRIES));
    }

    if (aux == 0)
        return p;
    return NULL;

}
