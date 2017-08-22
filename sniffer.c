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


#define BUFFER_SIZE 2000

int main(int argc, char const *argv[])
{
	if(argc < 2){
		perror("Falta argumento de numero de paquetes a capturar")
		exit(-1);
	}

	char buff[BUFFER_SIZE];

	int socket_raw;
	int flag;
	int nPaquetes=atoi(argv[0]);

	flag=create_socket(socket_raw);
	if(flag == 0)	//si flag es 0 entoces el socket se creo bien
	{
		int len=recvfrom(sock,&buff,sizeof(buff),0,&saddr,&addr_size);
		procesar_trama(&buff,BUFFER_SIZE)
	}
		
	return 0;
}

int procesar_trama(char *buffer, int buffer_size){
	
}

int create_socket(int *socket)
{
	struct sockaddr_in source,dest;
	struct sockaddr saddr;
	struct ifreq ethreq;

	int sock= socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(sock < 0){
		perror("Error al abrir el socket %d",sock)
		return -1;
	}

	strncpy(ethreq.ifr_name,"eno1",IFNAMSIZ);

	int io=ioctl(sock,SIOCGIFFLAGS,&ethreq);
	if(io<0)
	{
		perror("Error al obtener banderas %d",io)
		return -1;
	}

	ethreq.ifr_flags |=IFF_PROMISC;


	io=ioctl(sock,SIOCSIFFLAGS,&ethreq);
	if(io<0)
	{
		perror("Error al asignar banderas %d",io)
		return -1;
	}

	*socket=sock;
	return 0
	
}
