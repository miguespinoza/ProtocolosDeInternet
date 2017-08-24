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

int create_socket(); //funcion para crear el socket y poner tarjeta en modo promiscuo
int procesar_trama(unsigned char *buffer, int buffer_size, int nTrama); //procesar la trama capturada

int socket_raw; //unico socket en todo el programa
FILE *file;

int main(int argc, char const *argv[])
{
	struct sockaddr saddr;
	if(argc < 2){ //como argumento recibe el numero de paquetes a capturar, si no lo tiene termina el programa
		perror("Falta argumento de numero de paquetes a capturar");
		exit(-1);
	}
	
	file=fopen("log.txt","w"); //abrir el archivo con permisos de escritura (sobreescribe el archivo si ya existe)
    if(file==NULL){
        perror("Error al abrir el archivo: ");
        return -1;
	}
	
	//buffer para capturar datos, puede ser buffer normal 
	char *buff=(unsigned char *) malloc(BUFFER_SIZE);

	int flag; //bandera para ver si se creo bien el socket 0 -> bien  -1 ->error
	int nPaquetes=atoi(argv[1]);
	flag=create_socket();
	if(flag == 0)	//si flag es 0 entoces el socket se creo bien
	{
		int addr_size =sizeof(saddr);
		for(int index=0;index<nPaquetes;index++){//recibe los n paquetes que se indico en el paquete
			int len=recvfrom(socket_raw,buff,BUFFER_SIZE,0,&saddr,&addr_size);//recibe los paquetes
			if(len<0){ //si longitud de paquete es < 0 entonces hubo error
				perror("error al recibir paquete"); //funcion que imprime un mensaje y el mensaje "oficial" de error
				exit(-1);
			}	

			procesar_trama(buff,len,index); //manda a procesar la trama
		}
		

	}
	else{
		perror("error al abrir el socket");
	}
	fclose(file);
	return 0;
}

int procesar_trama(unsigned char *buffer, int buffer_size, int nTrama)
{
	/* TODO tareas a realizar
	MAC fuente 
	Dirección MAC destino 
	Longitud de la trama 
	Longitud de carga útil (datos y relleno) 
	Determinar si la dirección de destino es una dirección de
		 unidifusión, difusión o multidifusió*/

	uint16_t type = 0;
	memcpy(&type,&buffer[12],sizeof(uint16_t)); //copia los bytes 12 y 13 de la trama al campo type
	type = (type>>8) | (type<<8); //invierte los bytes ya que estan en Big endian. Ej. de 11110000 pasan a 00001111
	unsigned char  *destino=(unsigned char *) malloc(10);
	unsigned char  *origen=(unsigned char *) malloc(10);

	fprintf(file,"\nTrama %d \n",nTrama);
	printf("\nTrama %d \n",nTrama);

	for(int i=0;i<buffer_size; i++){
		printf("%x",buffer[i]);
	}
	printf("\n");

	if(type <= 0x05dc){ //si es de 0 a 05dc es ethernet II
		printf("ETHERNET II \n ");
		fprintf(file,"ETHERNET II");
	}
	else{
		switch(type)
		{
			case 0x0800:
				printf("IPv4 \n");
				fprintf(file,"Protocolo: IPv4\n");
				break;
			case 0x86dd:
				printf("IPv6 \n");	
				fprintf(file,"Protocolo: IPv6\n");
				break;
			case 0x0806:
				printf("ARP \n");
				fprintf(file,"Protocolo: ARP\n");
				break;
			case 0x8808:
				printf("Control de flujo ethernet \n ");
				fprintf(file,"Protocolo: Control de flujo ethernet\n");
				break;
			case 0x88e5: 
				printf("Seguridad MAC \n");
				fprintf(file,"Protocolo: Seguridad MAC\n");
				break;
			default:
				printf("Otro \n");
				fprintf(file,"Protocolo: Otro\n");
		}
		printf("Destino: ");
		fprintf(file,"Destino: ");
		memcpy(destino,&buffer[0],6); //copia la mac destino de la trama al string buffer, desde el inicio de la trama copia 6 bytes
		for(int i=0;i<6;i++){
			printf("%x",destino[i]);	//imprime en consola dir destino byte por byte
			fprintf(file,"%x",destino[i]);
		}
		printf("\n");
		fprintf(file,"\n");

		printf("Origen: ");
		fprintf(file,"Origen: ");
		memcpy(origen,&buffer[6],6); //MAC origen, copia la mac origen de trama a destino, el offset es 6
		for(int i=0;i<6;i++){
			printf("%x",origen[i]);	//imprime origen byte por byre
			fprintf(file,"%x",origen[i]);
		}
		printf("\n");
		fprintf(file,"\n");

		fprintf(file,"Longitud: %d \n",buffer_size); //longutud de trama
		printf("Longitud %d \n",buffer_size);
		fprintf(file,"Longitud payload: %d \n",buffer_size-14); //longitud de payload igual -14
		printf("Longitud payload: %d \n",buffer_size-14);

		uint16_t tipoMAC;				//UNIDIFUSION O MULTIDIFUSION
		memcpy(&tipoMAC,&buffer[0],1);	//copia el primer byte de la trama a un entero
		if(tipoMAC << 7 == 1){	//saca el ultimo bit del primer byte, 
			printf(" Multidifusion\n");
			fprintf(file,"Multidifusion\n");	//si es 1 es multidifusion
		}
		else{
			printf("Unidifusion\n");
			fprintf(file,"Unidifusion\n");	//si es 0 es unidifusion
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
