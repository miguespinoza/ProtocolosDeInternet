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

#define PROTO_ARP 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define BUF_SIZE 60


int create_socket();
void getLocalMac(unsigned char *MAC_str);

void getLocalIp(unsigned char *ip_str);
int broadcastSock;


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
    
    create_socket();

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
    #define MACLEN 6
    #define IPLEN 4
    struct sockaddr saddr;
    msgARP arp;
    char *ip ;
    unsigned char *local_ip, *local_mac;
    local_ip=(unsigned char*) malloc(sizeof(unsigned char)*8);
    local_mac=(unsigned char*) malloc(sizeof(unsigned char)*12);
    ip= (char *) ptr;
    getLocalMac(local_mac);
    getLocalIp(local_ip);
    for (int i=0; i<MACLEN; i++){
        sprintf(arp.origenMAC,"%02X",local_mac[i]);
    }
    printf("\n");

    
    memcpy(&arp.destinoIP,&ip,sizeof(char)*4);
    memcpy(&arp.origenIP,&local_ip,sizeof(char)*4);
    ;
    for(int i=0(;i<MACLEN;i++){
        arp.destinoMAC[i]='0';
    }    
    
    arp.tipoProtocolo=htons(ETH_P_IP);
    arp.longitudHardware=MAC_LENGTH;
    arp.longitudProtocolo=IPV4_LENGTH;
    ARP.tipoMensaje = htons(ARPOP_REQUEST);
    ARP.tipoEthernet= htons(ETH_P_ARP);
    arp.tipoHardware= htons(ARPHRD_ETHER);

    printf("Mensaje ARP generado para IP: %d.%d.%d.%d\n",ARP.destinoIP[0],ARP.destinoIP[1],ARP.destinoIP[2],ARP.destinoIP[3]);

    bzero(&saddr,sizeof(saddr));
    strcpy(saddr,)
    
    memset(&s, '\0', sizeof(struct sockaddr_in));
    s.sin_family = AF_INET;
    s.sin_addr.s_addr = htonl(INADDR_BROADCAST); 

    if(sendto(broadcastSock, &arp, sizeof(msgARP), 0, (struct sockaddr *)&s, sizeof(struct sockaddr_in)) < 0)
    perror("sendto"); 
    if(sendto(conexion     ,&ARP,sizeof(ARP),0,(struct sockaddr *)&saddr,sizeof(saddr))<0){
        perror("Error al envío\n");
            close(conexion);
        exit(1);
    }
}

void getLocalIp(unsigned char *ip_str){
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eno1", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    unsigned char *local_ip;
    local_ip=(unsigned char*) malloc(sizeof(unsigned char)*8);

    struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    sprintf(&ip_str[0],"%02x\n",(ipaddr->sin_addr.s_addr>>0) &0x000000ff);
    sprintf(&ip_str[2],"%02x\n",(ipaddr->sin_addr.s_addr>>8) &0x000000ff);
    sprintf(&ip_str[4],"%02x\n",(ipaddr->sin_addr.s_addr>>16) &0x000000ff);
    sprintf(&ip_str[6],"%02x\n",(ipaddr->sin_addr.s_addr>>24) &0x000000ff);
}

void getLocalMac(unsigned char *MAC_str)
{
    #define MACLEN 6
    
    int s,i;
    struct ifreq ifr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, "eno1");
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

	broadcastSock= socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ARP));
	if(broadcastSock < 0){
		perror("Error al abrir el socket ");
		return -1;
    }
    
    setsockopt(broadcastSock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));


	return 0;
	
}