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

#include<netinet/ip_icmp.h>   //Provides declarations for icmp header
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header


#define BUFFER_SIZE 2000

int create_socket();
int procesar_trama(unsigned char *buffer, int buffer_size);

int socket_raw;


int main(int argc, char const *argv[])
{
	struct sockaddr saddr;
	if(argc < 2){
		perror("Falta argumento de numero de paquetes a capturar");
		exit(-1);
	}
	char *buff=(unsigned char *) malloc(sizeof(unsigned char)*BUFFER_SIZE);
	int flag;
	int nPaquetes=atoi(argv[1]);
	flag=create_socket();
	if(flag == 0)	//si flag es 0 entoces el socket se creo bien
	{
		int addr_size =sizeof(saddr);
		for(int index=0;index<nPaquetes;index++){
			int len=recvfrom(socket_raw,buff,BUFFER_SIZE,0,&saddr,&addr_size);
			if(len<0){
				perror("error al recibir paquete");
				exit(-1);
			}	
			printf("l: %d\n",len);

			procesar_trama(buff,BUFFER_SIZE);
		}
		

	}
	else{
		perror("error al abrir el socket");
	}
	return 0;
}

int procesar_trama(unsigned char *buffer, int buffer_size)
{
	/*MAC fuente 
	Dirección MAC destino 
	Longitud de la trama 
	Longitud de carga útil (datos y relleno) 
	Determinar si la dirección de destino es una dirección de
		 unidifusión, difusión o multidifusió*/

	
	uint16_t type = 0;

	memcpy(&type,&buffer[12],sizeof(uint16_t));
	type = (type>>8) | (type<<8);


	if(type <= 0x05dc){
		printf("ETHERNET II -- ");
	}
	else{
		switch(type)
		{
			case 0x0800:
				printf("IPv4 ");
				break;
			case 0x86dd:
				printf("IPv6 -- ");	
				break;
			case 0x0806:
				printf("ARP -- ");
				break;
			case 0x8808:
				printf("Control de flujo ethernet -- ");
				break;
			case 0x88e5: 
				printf("Seguridad MAC -- ");
				break;
			default:
			printf("Otro -- ");

		}
	}

}

int create_socket()
{
	struct sockaddr_in source,dest;
	
	struct ifreq ethreq;

	socket_raw= socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(socket_raw < 0){
		perror("Error al abrir el socket ");
		return -1;
	}

	strncpy(ethreq.ifr_name,"eno1",IFNAMSIZ);

	int io=ioctl(socket_raw,SIOCGIFFLAGS,&ethreq);
	if(io<0)
	{
		perror("Error al obtener banderas ");
		return -1;
	}

	ethreq.ifr_flags |=IFF_PROMISC;


	io=ioctl(socket_raw,SIOCSIFFLAGS,&ethreq);
	if(io<0)
	{
		perror("Error al asignar banderas ");
		return -1;
	}

	return 0;
	
}
