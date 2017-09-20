#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <stdint.h>
#include <sys/types.h>
#include <net/if.h>
#include<netinet/ip.h> 
#include <pthread.h>


int create_socket();

int sock;

typedef struct ARP_struct{
	unsigned char destinoEthernet[6];      /*Dirección de difusión 0xFF*/
	unsigned char origenEthernet[6];       /*Dirección MAC del transmisor*/
	unsigned short tipoEthernet;             /*Tipo de mensaje en la trama Ethernet*/
	unsigned short tipoHardware;           /*Tipo de hardware utilizado para difundir el 
						mensaje ARP (Ethernet)*/
	unsigned short tipoProtocolo;          /*Tipo de protocolo de red utilizado para
 difundir el mensaje ARP (IP)*/
    unsigned char longitudHardware;  /*Tamaño de direcciones de hardware (6bytes)*/
    unsigned char longitudProtocolo;  /*Tamaño de direcciones del protocolo (4bytes)*/
    unsigned short tipoMensaje;          /* Solicitud o respuesta*/
	unsigned char origenMAC[6];         /*Dirección MAC del transmisor*/
	unsigned char origenIP[4];             /*Dirección IP del transmisor*/
	unsigned char destinoMAC[6];  /*Dirección MAC del receptor (dirección solicitada)*/
unsigned char destinoIP[4];         /*Dirección IP del receptor (dato de entrada)*/
} msgARP;
void* ARP_process(void *ptr);

int main(int argc, char const *argv[])
{
    int nip = atoi(argv[1]);
    pthread_t *threads= (pthread_t*)malloc(sizeof(int)*nip);
    int threadId[nip];
    const char **ips;
    ips=(const char**)malloc(sizeof(char)*12);
    for(int i=0;i<12;i++){
        ips[i]=(const char*)malloc(sizeof(char)*nip);
    }
    
    for(int i=0;i<nip;i++){
        ips[i]=argv[i+2];
    }
    
    //create_socket();

    for(int i=0;i<nip;i++){
        
        threadId[i]=pthread_create(&threads[i],NULL,ARP_process,(void*)ips[i]);
        printf("%d: %d\n",threadId[i],i);
        if(threadId[i])
             {
                printf("Error - pthread_create() return code: %d\n",threadId[i]);
                exit(EXIT_FAILURE);
             }
        
    }
    for(int i=0;i<nip;i++){
        pthread_join( threadId[i], NULL);
        printf("%d: %d\n",threadId[i],i);
    }
    

    
 
}


void* ARP_process(void *ptr){
    printf("hilo");
    char *ip ;
    ip= (char *) ptr;
    printf("%s\n",ip);

}



int create_socket()
{
	struct sockaddr_in source,dest;
    struct ifreq ethreq;
    int optval;

	sock= socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ARP));
	if(sock < 0){
		perror("Error al abrir el socket ");
		return -1;
    }
    
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));


	return 0;
	
}