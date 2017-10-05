#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <stdint.h>
#include <sys/types.h>
#include <net/if.h>
#include<netinet/ip.h> 
#include <pthread.h>
#include <unistd.h>

int create_socket(int *);


char device[] = "eno1";
//char device[] = "wlp2s0";


typedef struct ARP_struct{
	unsigned char destinoEthernet[6];      /*Dirección de difusión 0xFF*/
	unsigned char origenEthernet[6];       /*Dirección MAC del transmisor*/
	unsigned short tipoEthernet;             /*Tipo de mensaje en la trama Ethernet*/
	unsigned short tipoHardware;           /*Tipo de hardware utilizado para difundir el mensaje ARP (Ethernet)*/
	unsigned short tipoProtocolo;          /*Tipo de protocolo de red utilizado paradifundir el mensaje ARP (IP)*/
    unsigned char longitudHardware;  /*Tamaño de direcciones de hardware (6bytes)*/
    unsigned char longitudProtocolo;  /*Tamaño de direcciones del protocolo (4bytes)*/
    unsigned short tipoMensaje;          /* Solicitud o respuesta*/
	unsigned char origenMAC[6];         /*Dirección MAC del transmisor*/
	unsigned char origenIP[4];             /*Dirección IP del transmisor*/
	unsigned char destinoMAC[6];  /*Dirección MAC del receptor (dirección solicitada)*/
unsigned char destinoIP[4];         /*Dirección IP del receptor (dato de entrada)*/
} msgARP;

void getLocalData(struct ifreq *NetworkDevice, msgARP *ARP);
void* ARP_process(void *ptr);

int main(int argc, char const *argv[])
{
    int nip = atoi(argv[1]);
    pthread_t *threads= (pthread_t*)malloc(sizeof(int)*nip);
    int threadId[nip];
    
    for(int i=1;i<=nip;i++){
       // printf("ip: %s\n",argv[i+1] );
        threadId[i]=pthread_create(&threads[i-1],NULL,ARP_process,(void*) argv[i+1]);
        if(threadId[i])
             {
                printf("Error - pthread_create() return code: %d\n",threadId[i]);
                exit(EXIT_FAILURE);
             }
        
    }
    for(int i=1;i<=nip;i++){
        pthread_join( threads[i-1], NULL);
    }
    

    
 
}


void* ARP_process(void *ptr){
    #define HWADDR_len 6
    #define IP_len 4
    msgARP ARP;
    const char *ipToSolve;
    ipToSolve=(const char *)ptr;
    printf("inicio de proceso ARP IP: %s\n",ipToSolve );
    struct ifreq NetworkDevice;
    struct sockaddr saddr; 
    int sock;
    create_socket(&sock);
    unsigned char ipdos[4];
    unsigned char *local_ip, *local_mac;
    local_ip=(unsigned char*) malloc(sizeof(unsigned char)*8);
    local_mac=(unsigned char*) malloc(sizeof(unsigned char)*12);
    

   // getLocalData(&NetworkDevice, &ARP);
    strncpy(NetworkDevice.ifr_name,device,IFNAMSIZ);
    if(ioctl(sock, SIOCGIFHWADDR, &NetworkDevice) < 0){
        perror("Error al obtener información del hadware\n");
        close(sock);
        exit(1);
    } 

    bcopy(&NetworkDevice.ifr_hwaddr.sa_data,&ARP.origenMAC,6);
    bcopy(&NetworkDevice.ifr_hwaddr.sa_data,&ARP.origenEthernet,6);
    
    
    strcpy(NetworkDevice.ifr_name,device);
    
   // printf("\t MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",ARP.origenMAC[0],ARP.origenMAC[1],ARP.origenMAC[2],ARP.origenMAC[3],ARP.origenMAC[4],ARP.origenMAC[5] );

    if(ioctl(sock, SIOCGIFADDR, &NetworkDevice) < 0){
        perror("Error al reasignar bandera  \n");
        close(sock);
        exit(1);
    }
    bcopy(&NetworkDevice.ifr_addr.sa_data[2],&ARP.origenIP,4);
   // printf(" origen IP: %d.%d.%d.%d\n",ARP.origenIP[0],ARP.origenIP[1],ARP.origenIP[2],ARP.origenIP[3]);




// ORIGEN ARP 
    ARP.longitudHardware = 6;
    ARP.longitudProtocolo = 4;
    ARP.tipoProtocolo = htons(ETH_P_IP);
    ARP.tipoHardware = htons(ARPHRD_ETHER);
    ARP.tipoMensaje = htons(ARPOP_REQUEST);
    bcopy(&NetworkDevice.ifr_addr.sa_data[2],&ARP.origenIP,4);
    bzero(&ARP.destinoMAC,7);
    inet_aton(ipToSolve,ARP.destinoIP);
    strncpy(ipdos,ARP.destinoIP,4);
        // ETHERNET 
    memset(&ARP.destinoEthernet,0xff,6);
    ARP.tipoEthernet= htons(ETH_P_ARP);
        bzero(&saddr,sizeof(saddr));
    strcpy(saddr.sa_data,device);
    printf("Mensaje ARP generado para IP: %d.%d.%d.%d\n",ARP.destinoIP[0],ARP.destinoIP[1],ARP.destinoIP[2],ARP.destinoIP[3]);
    if(sendto(sock,&ARP,sizeof(ARP),0,(struct sockaddr *)&saddr,sizeof(saddr))<0){
        perror("Error al envío\n");
            close(sock);
        exit(1);
    }
    bzero(&ARP, sizeof(ARP));
    bzero((struct sockaddr*)&saddr, sizeof(saddr));
    int n =  sizeof(saddr);
    int flag = 0; 
    while(flag==0){
        if (recvfrom(sock, &ARP, sizeof(ARP), 0, (struct sockaddr*)&ARP,&n) < 0) {
            perror("Error recibir mensaje");
            exit(1);
        }
        printf("\t IP:  %d.%d.%d.%d\n",(int)ARP.origenIP[0],(int)ARP.origenIP[1],(int)ARP.origenIP[2],(int)ARP.origenIP[3]);
        printf("\t MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",ARP.origenMAC[0],ARP.origenMAC[1],ARP.origenMAC[2],ARP.origenMAC[3],ARP.origenMAC[4],ARP.origenMAC[5] );
        flag = 1;
        if ((ntohs(ARP.tipoMensaje) == ARPOP_REPLY) && !strncmp(ipToSolve,ARP.origenIP,4)) {
           printf("*** RESPUESTA ARP ***\n");
           printf("\t IP:  %d.%d.%d.%d\n MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",(int)ARP.origenIP[0],(int)ARP.origenIP[1],(int)ARP.origenIP[2],(int)ARP.origenIP[3],ARP.origenMAC[0],ARP.origenMAC[1],ARP.origenMAC[2],ARP.origenMAC[3],ARP.origenMAC[4],ARP.origenMAC[5]);
        flag = 1;
        }
    }
    close(sock);
}

void getLocalData(struct ifreq *NetworkDevice, msgARP *ARP){
    int s,i;
    struct ifreq ifr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, "eno1");
    if(ioctl(s, SIOCGIFHWADDR, &ifr) < 0){
            perror("Error al obtener información del hadware\n");
            close(s);
            exit(1);    
    } 
    close(s);




    bcopy(NetworkDevice->ifr_hwaddr.sa_data,ARP->origenMAC,6);
    bcopy(NetworkDevice->ifr_hwaddr.sa_data,ARP->origenEthernet,6);
    strcpy(NetworkDevice->ifr_name,device);
    printf(" origen IP: %d.%d.%d.%d\n",ARP->origenIP[0],ARP->origenIP[1],ARP->origenIP[2],ARP->origenIP[3]);
    printf("\t MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",ARP->origenMAC[0],ARP->origenMAC[1],ARP->origenMAC[2],ARP->origenMAC[3],ARP->origenMAC[4],ARP->origenMAC[5] );
    
    close(s);
}





int create_socket(int *sock)
{

    int opval;

	*sock= socket(PF_INET,SOCK_PACKET,htons(ETH_P_ARP));
	if(*sock < 0){
		perror("Error al abrir el socket ");
		return -1;
    }
    
    if( setsockopt(*sock,SOL_SOCKET,SO_BROADCAST,&opval,sizeof(opval)) <0 ){
            perror("Error al configurar broadcast");
            exit(1);
        }

	return 0;
	
}