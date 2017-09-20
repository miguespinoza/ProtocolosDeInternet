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
void getLocalMac(unsigned char *MAC_str);

void getLocalIp(unsigned char *ip_str);
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
        if(threadId[i])
             {
                printf("Error - pthread_create() return code: %d\n",threadId[i]);
                exit(EXIT_FAILURE);
             }
        
    }
    for(int i=0;i<nip;i++){
        pthread_join( threads[i], NULL);
    }
    

    
 
}


void* ARP_process(void *ptr){
    #define HWADDR_len 6
    #define IP_len 4
    //printf("hilo");
    char *ip ;
    unsigned char *local_ip, *local_mac;
    local_ip=(unsigned char*) malloc(sizeof(unsigned char)*8);
    local_mac=(unsigned char*) malloc(sizeof(unsigned char)*12);
   // ip= (char *) ptr;

    printf("ip");
    getLocalMac(local_mac);
    getLocalIp(local_ip);
    for (int i=0; i<IP_len;i++)
        printf("%02X",local_ip[i]);
    printf("\nMAC");

    for (int i=0; i<HWADDR_len; i++)
        printf("%02X",local_mac[i]);
    printf("\n");
}

void getLocalIp(unsigned char *ip_str){
    int fd;
    struct ifreq ifr;
   
    fd = socket(AF_INET, SOCK_DGRAM, 0);
   
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
   
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "enp3s0", IFNAMSIZ-1);
   
    ioctl(fd, SIOCGIFADDR, &ifr);
   
    close(fd);
   
    /* display result */
    sprintf(ip_str,"%x\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    memcpy(ip_str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 6);
}

void getLocalMac(unsigned char *MAC_str)
{
    #define HWADDR_len 6
    #define IP_len 4
    int s,i;
    struct ifreq ifr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, "enp3s0");
    ioctl(s, SIOCGIFHWADDR, &ifr);
    close(s);
    

    memcpy(MAC_str, ifr.ifr_hwaddr.sa_data, 6);
}/*
typedef struct ARP_struct{
	unsigned char destinoEthernet[6];      //Dirección de difusión 0xFF
	unsigned char origenEthernet[6];       //Dirección MAC del transmisor
	unsigned short tipoEthernet;             //Tipo de mensaje en la trama Ethernet
	unsigned short tipoHardware;           //Tipo de hardware utilizado para difundir el mensaje ARP (Ethernet)
	unsigned short tipoProtocolo;          //Tipo de protocolo de red utilizado para
 difundir el mensaje ARP (IP)
    unsigned char longitudHardware;  //Tamaño de direcciones de hardware (6bytes)
    unsigned char longitudProtocolo;  //Tamaño de direcciones del protocolo (4bytes)
    unsigned short tipoMensaje;          // Solicitud o respuesta
	unsigned char origenMAC[6];         //Dirección MAC del transmisor
	unsigned char origenIP[4];             //Dirección IP del transmisor
	unsigned char destinoMAC[6];  //Dirección MAC del receptor (dirección solicitada)
unsigned char destinoIP[4];         //Dirección IP del receptor (dato de entrada)
} msgARP;*/



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