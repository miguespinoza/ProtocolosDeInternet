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

int procesar_trama(unsigned char *buffer, int buffer_size, int nTrama); 

int socket_raw; 
FILE *file;

int main(int argc, char const *argv[])
{
	struct sockaddr saddr;
	if(argc < 2){ 
		perror("Falta argumento de numero de paquetes a capturar");
		exit(-1);
	}
	
	file=fopen("log.txt","w"); //abrir el archivo con permisos de escritura (sobreescribe el archivo si ya existe)
    if(file==NULL){
        perror("Error al abrir el archivo: ");
        return -1;
	}
	
	//buffer para capturar datos, puede ser buffer normal 
	unsigned char *buff=(unsigned char *) malloc(BUFFER_SIZE);
	int flag; //bandera para ver si se creo bien el socket 0 -> bien  -1 ->error
	int nPaquetes=atoi(argv[1]);
	struct sockaddr_in source,dest;
	struct ifreq ethreq;

	socket_raw= socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(socket_raw < 0){
		perror("Error al abrir el socket ");
	}

	strncpy(ethreq.ifr_name,"eno1",IFNAMSIZ);

	int io=ioctl(socket_raw,SIOCGIFFLAGS,&ethreq);

	ethreq.ifr_flags |=IFF_PROMISC;

	io=ioctl(socket_raw,SIOCSIFFLAGS,&ethreq);


	int addr_size =sizeof(saddr);
	for(int index=0;index<nPaquetes;index++){//recibe los n paquetes que se indico en el paquete
		int len=recvfrom(socket_raw,buff,BUFFER_SIZE,0,&saddr,&addr_size);//recibe los paquetes
		if(len<0){ //si longitud de paquete es < 0 entonces hubo error
			perror("error al recibir paquete"); //funcion que imprime un mensaje y el mensaje "oficial" de error
			exit(-1);
		}	

		procesar_trama(buff,len,index); //manda a procesar la trama
	}
		
	fclose(file);
	return 0;
}

int procesar_trama(unsigned char *buffer, int buffer_size, int nTrama)
{

	uint16_t type = 0;
	memcpy(&type,&buffer[12],sizeof(uint16_t)); 
	type = (type>>8) | (type<<8); 
	unsigned char  *destino=(unsigned char *) malloc(10);
	unsigned char  *origen=(unsigned char *) malloc(10);

	fprintf(file,"\n----------------------------\nTrama %d \n",nTrama+1);
	printf("\n----------------------------\nTrama %d \n",nTrama+1);

	for(int i=0;i<buffer_size; i++){
		printf("%x",buffer[i]);
	}
	printf("\n");

	if(type <= 0x05dc){ 
		printf("Ethernet II \n ");
		fprintf(file,"Ethernet II");
	}
	else{
		switch(type)
		{
			case 0x0800:
				printf("Protocolo superior: IPv4 \n");
				fprintf(file,"Protocolo superior: IPv4\n");
				break;
			case 0x86dd:
				printf("Protocolo superior: IPv6 \n");	
				fprintf(file,"Protocolo superior: IPv6\n");
				break;
			case 0x0806:
				printf("Protocolo superior: ARP \n");
				fprintf(file,"Protocolo superior: ARP\n");
				break;
			case 0x8808:
				printf("Protocolo superior: Control de flujo ethernet \n ");
				fprintf(file,"Protocolo superior: Control de flujo ethernet\n");
				break;
			case 0x88e5: 
				printf("Protocolo superior: Seguridad MAC \n");
				fprintf(file,"Protocolo superior: Seguridad MAC\n");
				break;
			default:
				printf("Protocolo superior: Otro \n");
				fprintf(file,"Protocolo superior: Otro\n");
		}
		printf("Direccion estino: ");
		fprintf(file,"Direccion Destino: ");
		memcpy(destino,&buffer[0],6); //copia la mac destino de la trama al string buffer, desde el inicio de la trama copia 6 bytes
		for(int i=0;i<6;i++){
			printf("%x",destino[i]);	//imprime en consola dir destino byte por byte
			fprintf(file,"%x",destino[i]);
		}
		printf("\n");
		fprintf(file,"\n");

		printf("Direccion Origen: ");
		fprintf(file,"Direccion Origen: ");
		memcpy(origen,&buffer[6],6); //MAC origen, copia la mac origen de trama a destino, el offset es 6
		for(int i=0;i<6;i++){
			printf("%x",origen[i]);	//imprime origen byte por byre
			fprintf(file,"%x",origen[i]);
		}
		printf("\n");
		fprintf(file,"\n");

		fprintf(file,"Longitudde trama: %d \n",buffer_size); //longutud de trama
		printf("Longitud de trama: %d \n",buffer_size);
		fprintf(file,"Longitud de carga util: %d \n",buffer_size-14); //longitud de payload igual -14
		printf("Longitud de carga util: %d \n",buffer_size-14);

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

