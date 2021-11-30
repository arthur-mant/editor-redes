#include <bits/stdc++.h>

#ifndef COMMON
#define COMMON
#include "common.h"
#endif

#define BUFFERSIZE 65536

/*
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

//função do albini
//utilizar Loop Back (lo) como device
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
/*
int open_socket() {

    int sock_r;

    sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sock_r<0) {
        printf("error opening socket\n");
        return -1;
    }
    return sock_r;

}
*/
int receive_from_socket(int sock, unsigned char *buffer, struct sockaddr sadrr) {

    int buflen, saddr_len;

    saddr_len = sizeof(sadrr);

    buflen = recvfrom(sock, buffer, BUFFERSIZE, 0, &saddr, (socklen_t *)&saddr_len);

    if (buflen < 0) {
        printf("error reading recvfrom function\n");
        return -1;
    }
    return buflen;

}

unsigned char *empacota(unsigned char *buffer, int tamanho, int tipo, int destino, int origem, int sequencia) {

    //[inicio, destino origem tamanho, sequencia tipo, dados, paridade]

    unsigned char *packet;
    unsigned char inicio, dest_c, orig_c, tam_c, seq_c, tipo_c, paridade, tmp;

    packet = malloc(8+2+2+4+4+4+tam+8);

    inicio = 0b01111110;
    packet[0] = inicio;

    dest_c = (unsigned char) destino;
    orig_c = (unsigned char) origem;
    tam_c = (unsigned char) tamanho;
    packet[1] = (dest_c << (2+4)) | (orig_c << 4) | (tam_c);

    seq_c = (unsigned char) sequencia;
    tipo_c = (unsigned char) tipo;
    packet[2] = (seq_c << 4) | (tipo_c);

    tmp = tam_c ^ seq_c ^ tipo_c;

    for(int i=0; i<tamanho; i++) {
        packet[3+i] = buffer[i];
        tmp = tmp ^ buffer[i];
    }
    
    packet[3+tamanho] = tmp;

    //packet_len = 4+tamanho
    return packet;
}

int send_to_socket(int sock, unsigned char *buffer, int tam, int tipo, int destino, int origem) {

    int full_p, tam_leftover_p, index;

    full_p = tam/15;
    tam_leftover_p = tam % 15;

    for (int i=0; i<full_p; i++) {
        index = i % 16;
        empacota(buffer[15*i], 15, tipo, destino, origem, index);

    }

    if ((tam_leftover_p > 0) || (full_p == 0)) {
        empacota(buffer[15*full_p], tam_leftover_p, tipo, destino, origem, (index+1) % 16);


    }
}


