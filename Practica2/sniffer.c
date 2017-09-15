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
#include<netinet/ip.h>    //Provides declarations for ip header

typedef struct header_ip{ 
	unsigned short version;/*Versión y longitud de cabecera*/ 
	unsigned short longitudHeader;
	unsigned short tipoSer;/*Tipo de servicio*/ 
	unsigned char longTotal;  /*Longitud total del datagrama*/ 
	unsigned short IdentDgram;  /*Identificador de datagrama*/
	unsigned char banderaOffset;  /*Fragmentación*/ 
	unsigned short ttl; /*Tiempo de vida*/ 
	unsigned short protSup; /*Protocolo de capa superior*/ 
	unsigned short sumVer;  /*Suma de verificación*/ 
	unsigned char IPorigen[4];/*Dirección IP del transmisor*/ 
	unsigned char IPdestino[4];/*Dirección IP del receptor*/ 
}header_ip; 

void print_protocolo(unsigned short proto);

#define BUFFER_SIZE 2000 

int create_socket(); //funcion para crear el socket y poner tarjeta en modo promiscuo
int procesar_trama(unsigned char *buffer, int buffer_size, int nTrama); //procesar la trama capturada
int procesar_trama_ip(unsigned char *buffer, int buffer_size);
void parse_trama_ip(unsigned char *buffer, int buffer_size);

int socket_raw; //unico socket en todo el programa
FILE *file;
struct sockaddr_in source2;
char *aux;
header_ip *cabip;

int main(int argc, char const *argv[])
{
	cabip=(header_ip*)malloc(sizeof(header_ip));
	aux = (char*) malloc(50);
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

	if(type <= 0x05dc){ //si es de 0 a 05dc es ethernet II
		fprintf(file,"ETHERNET II");
	}
	else{
		switch(type)
		{
			case 0x0800:
				fprintf(file,"Protocolo: IPv4\n");
				procesar_trama_ip(buffer,buffer_size);
				break;
			case 0x86dd:
				fprintf(file,"Protocolo: IPv6\n");
				break;
			case 0x0806:
				fprintf(file,"Protocolo: ARP\n");
				break;
			case 0x8808:
				fprintf(file,"Protocolo: Control de flujo ethernet\n");
				break;
			case 0x88e5: 
				fprintf(file,"Protocolo: Seguridad MAC\n");
				break;
			default:
				fprintf(file,"Protocolo: Otro\n");
		}
	}

}

int procesar_trama_ip(unsigned char *buffer, int buffer_size){
	parse_trama_ip(buffer,buffer_size);

	fprintf(file,"  IP Origen : ");
	for(int i=0;i<4;i++){
		fprintf(file,"%02d.",cabip->IPorigen[i]);	
	//	fprintf(file,"%02x",cabip->IPdestino[i]);
	}

	fprintf(file,"\n");
	fprintf(file,"  IP Destino : ");
	for(int i=0;i<4;i++){
		fprintf(file,"%02d.",cabip->IPdestino[i]);	
	//	fprintf(file,"%02x",cabip->IPdestino[i]);
	}
	fprintf(file,"\n");

	fprintf(file,"  Version de protocolo IP  : %d\n",cabip->version);

	int lon=cabip->longitudHeader*4;
	fprintf(file,"  Longitud de cabecera :  %d Bytes\n",lon);

	fprintf(file,"  Longitud de paquete : %d Bytes\n", buffer_size);

	fprintf(file,"  Identificador    : %d\n",cabip->IdentDgram);

	fprintf(file,"  Tiempo de vida      : %d\n",cabip->ttl);

	fprintf(file,"  Protocolo superior : %x\n",cabip->protSup);
	print_protocolo(cabip->protSup);

	if(lon == 20)
		fprintf(file,"  Sin opciones.\n");
	else
		fprintf(file,"  Con opciones.\n");
	
	fprintf(file,"  Longitud de carga util : %d Bytes\n",buffer_size-lon);

	short prece = cabip->tipoSer & 224;
	prece = prece >>5;
	if(prece == 0)
		fprintf(file,"  Bits de precedencia\n");
	else	
		fprintf(file,"  Servicio diferenciado\n");
	
	short frag = cabip->banderaOffset && 0b1110000000000000;
	frag = frag >>13;
	if(frag >= 0b010)
		fprintf(file,"  No fragmentado\n");
	else
		if(frag == 0b000)
			fprintf(file,"  Ultimo fragmento\n");
		if(frag == 0b001)
			fprintf(file,"  Fragmento Intermedio\n");

	short by= cabip->banderaOffset && 0b0001111111111111;
	fprintf(file,"  Primer byte : %d\n",by);
		
}

void print_protocolo(unsigned short proto){
	switch(proto){
		case 0x01:
		fprintf(file,"  MPv4\n");
		break;
		case 0x02:
		fprintf(file,"  IGMP\n");
		break;
		case 0x04:
		fprintf(file,"  IP\n");
		break;
		case 0x06:
		fprintf(file,"  TCP\n");
		break;
		case 0x11:
		fprintf(file,"  UDP\n");
		break;
		case 0x29:
		fprintf(file,"  IPv6\n");
		break;
		case 0x59:
		fprintf(file,"  OSPF\n");
		break;


	}
}



void parse_trama_ip(unsigned char *buffer, int buffer_size){
	uint temporal;

	memcpy(&temporal,&buffer[14],1);
	cabip->version = temporal & 240;
	cabip->version = cabip->version >>4;
	cabip->longitudHeader = temporal & 15;

	//Tipo de servicio 
	memcpy(&cabip->tipoSer,&buffer[15],1);

	//Longitud total del datagrama
	memcpy(&cabip->longTotal,&buffer[16],2);

	//Identificador de datagrama
	memcpy(&cabip->IdentDgram,&buffer[18],2);

	//Fragmentación
	memcpy(&cabip->banderaOffset,&buffer[20],2);
			
	//Tiempo de vida
	memcpy(&cabip->ttl,&buffer[22],1);

	//Protocolo de capa superior 
	memcpy(&cabip->protSup,&buffer[23],1);

	memcpy(&cabip->sumVer,&buffer[24],2);

	memcpy(&cabip->IPorigen,&buffer[26],4); //copia la direccion IP Origen de 4 bytes
	

	//Dirección IP del receptor

	memcpy(&cabip->IPdestino,&buffer[30],4); //copia la direccion IP Destino de 4 bytes
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

	strncpy(ethreq.ifr_name,"enp3s0",IFNAMSIZ);

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
