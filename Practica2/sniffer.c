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

struct ip_header {
	unsigned short version;  /*Versión y longitud de cabecera*/
	unsigned short longitudHeader;
 	unsigned short tipoSer;  /*Tipo de servicio*/
 	unsigned char longTotal;  /*Longitud total del datagrama*/
	unsigned char IdentDgram;  /*Identificador de datagrama*/
	unsigned char banderaOffset;  /*Fragmentación*/
	unsigned short ttl; /*Tiempo de vida*/
 	unsigned short protSup; /*Protocolo de capa superior*/
	unsigned char sumVer;  /*Suma de verificación*/
	unsigned char IPorigen[4];  /*Dirección IP del transmisor*/
	unsigned char IPdestino[4];  /*Dirección IP del receptor*/
};


#define BUFFER_SIZE 2000 

int create_socket(); //funcion para crear el socket y poner tarjeta en modo promiscuo
int procesar_trama(unsigned char *buffer, int buffer_size, int nTrama); //procesar la trama capturada
int procesar_trama_ip(unsigned char *buffer, int buffer_size);

int socket_raw; //unico socket en todo el programa
FILE *file;
struct sockaddr_in source2;
struct header = struct ip_header;

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
		fprintf(file,"Destino: ");
		memcpy(destino,&buffer[0],6); //copia la mac destino de la trama al string buffer, desde el inicio de la trama copia 6 bytes
		for(int i=0;i<6;i++){

			fprintf(file,"%x",destino[i]);
		}

		fprintf(file,"\n");


		fprintf(file,"Origen: ");
		memcpy(origen,&buffer[6],6); //MAC origen, copia la mac origen de trama a destino, el offset es 6
		for(int i=0;i<6;i++){

			fprintf(file,"%x",origen[i]);
		}

		fprintf(file,"\n");

		fprintf(file,"Longitud: %d \n",buffer_size); //longutud de trama

		fprintf(file,"Longitud payload: %d \n",buffer_size-14); //longitud de payload igual -14


		uint16_t tipoMAC;				//UNIDIFUSION O MULTIDIFUSION
		memcpy(&tipoMAC,&buffer[0],1);	//copia el primer byte de la trama a un entero
		if(tipoMAC << 7 == 1){	//saca el ultimo bit del primer byte, 

			fprintf(file,"Multidifusion\n");	//si es 1 es multidifusion
		}
		else{

			fprintf(file,"Unidifusion\n");	//si es 0 es unidifusion
		}
	}

}

int procesar_trama_ip(unsigned char *buffer, int buffer_size){
	struct iphdr *iph = (struct iphdr *)buffer;
	
	struct sockaddr_in pkt_source,pkt_dest;

	memset(&pkt_source, 0, sizeof(pkt_source));
	pkt_source.sin_addr.s_addr = iph->saddr;
		
	memset(&pkt_dest, 0, sizeof(pkt_dest));
	pkt_dest.sin_addr.s_addr = iph->daddr;

	fprintf(file,"  IP origen       : %s\n",inet_ntoa(pkt_source.sin_addr));
    fprintf(file,"  IP destino   : %s\n",inet_ntoa(pkt_dest.sin_addr));
	fprintf(file,"  Version IP        : %d\n",(unsigned int)iph->version);
	fprintf(file,"  Longitud de cabecera :  %d Bytes\n",((unsigned int)(iph->ihl))*4);
	fprintf(file,"  Longitud de paquete : %d Bytes\n", buffer_size);
	fprintf(file,"  Identificador    : %d\n",ntohs(iph->id));
	fprintf(file,"  Tiempo de vida      : %d\n",(unsigned int)iph->ttl);
	fprintf(file,"  Protocolo superior : %d\n",(unsigned int)iph->protocol);
	

		
}
void parse_trama_ip(unsigned char *buffer, int buffer_size){
	memcpy(&header.version,&buffer[0],3);
	memcpy(&header.longitudHeader,&buffer[4],7);
	memcpy(&header.tipoSer,&buffer[8],13);
	memcpy(&header.longTotal,&buffer[16],31);
	memcpy(&header.IdentDgram,&buffer[32],47);
	memcpy(&header.banderaOffset,&buffer[48],50);
	memcpy(&header.banderaOffset,&buffer[64],71);
	memcpy(&header.protSup,&buffer[72],79);
	memcpy(&header.IPorigen[0],&buffer[96],103);
	memcpy(&header.IPorigen[1],&buffer[104],111);
	memcpy(&header.IPorigen[2],&buffer[112],119);
	memcpy(&header.IPorigen[3],&buffer[120],127);
	memcpy(&header.IPdestino[0],&buffer[128],135);
	memcpy(&header.IPdestino[1],&buffer[136],143);
	memcpy(&header.IPdestino[2],&buffer[144],151);
	memcpy(&header.IPdestino[3],&buffer[152],159);	
}
struct ip_header {
	unsigned short version;  /*Versión y longitud de cabecera*/
	unsigned short longitudHeader;
 	unsigned short tipoSer;  /*Tipo de servicio*/
 	unsigned char longTotal;  /*Longitud total del datagrama*/
	unsigned char IdentDgram;  /*Identificador de datagrama*/
	unsigned char banderaOffset;  /*Fragmentación*/
	unsigned short ttl; /*Tiempo de vida*/
 	unsigned short protSup; /*Protocolo de capa superior*/
	unsigned char IPorigen[4];  /*Dirección IP del transmisor*/
	unsigned char IPdestino[4];  /*Dirección IP del receptor*/
};

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
