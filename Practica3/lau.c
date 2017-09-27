#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
struct msgARP{
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
};

int main( int argc,char *argv[] ){

	struct msgARP ARP;
	struct ifreq NetworkDevice;  /* is used to configure network devices */
	struct sockaddr saddr; 		
	char device[] = "wlp6s0";
	unsigned char ip[4];
  /*Auxiliares*/
		unsigned char auxorigenEthernet[6];
		unsigned char auxorigenMAC[6]; 

	int conexion = socket(PF_INET,SOCK_PACKET,htons(ETH_P_ARP));
	if(conexion < 0){
		perror("Error al crear socket\n");
		exit(1);	
	}
	//printf("Socket creado\n");
	int cont = 0;
	while(cont < 2){
		cont ++;
		int opval = 1;
		if( setsockopt(conexion,SOL_SOCKET,SO_BROADCAST,&opval,sizeof(opval)) <0 ){
			perror("Error al configurar broadcast");
			exit(1);
		}
		//printf("Broadcast activado\n");
		strncpy(NetworkDevice.ifr_name,device,IFNAMSIZ);
		//printf("Nombre de dispositivo: %s\n",NetworkDevice.ifr_name );
			
		if(ioctl(conexion, SIOCGIFHWADDR, &NetworkDevice) < 0){
			perror("Error al obtener información del hadware\n");
			close(conexion);
			exit(1);
		} 

		bcopy(&NetworkDevice.ifr_hwaddr.sa_data,&ARP.origenMAC,6);
		bcopy(&NetworkDevice.ifr_hwaddr.sa_data,&ARP.origenEthernet,6);
			bcopy(&NetworkDevice.ifr_hwaddr.sa_data,&auxorigenMAC,6);
		bcopy(&NetworkDevice.ifr_hwaddr.sa_data,&auxorigenEthernet,6);
		strcpy(NetworkDevice.ifr_name,device);

		if(ioctl(conexion, SIOCGIFADDR, &NetworkDevice) < 0){
			perror("Error al reasignar bandera  \n");
			close(conexion);
			exit(1);
		}


		/* ORIGEN ARP */
		ARP.longitudHardware = 6;
		ARP.longitudProtocolo = 4;
		ARP.tipoProtocolo = htons(ETH_P_IP);
		ARP.tipoHardware = htons(ARPHRD_ETHER);
		ARP.tipoMensaje = htons(ARPOP_REQUEST);
		bcopy(&NetworkDevice.ifr_addr.sa_data[2],&ARP.origenIP,4);
		bzero(&ARP.destinoMAC,7);
			inet_aton(argv[cont],ARP.destinoIP);
		strncpy(ip,ARP.destinoIP,4);
		/* ETHERNET */
		memset(&ARP.destinoEthernet,0xff,6);
		ARP.tipoEthernet= htons(ETH_P_ARP);

		bzero(&saddr,sizeof(saddr));
	    strcpy(saddr.sa_data,device);
		printf("Mensaje ARP generado para IP: %d.%d.%d.%d\n",ARP.destinoIP[0],ARP.destinoIP[1],ARP.destinoIP[2],ARP.destinoIP[3]);
	   /*printf("\tHardware: %x\n",ARP.tipoHardware);
	    printf("\tProtocolo: %x\n",ARP.tipoProtocolo);
	    printf("\tLongitud de hardware: %x\n",ARP.longitudHardware);
	    printf("\tLongitud de protocolo: %x\n",ARP.longitudProtocolo);
	    printf("\tTipo de mensaje: %x\n",ARP.tipoMensaje);
	    printf("\tMAC ORIGEN: %x:%x:%x:%x:%x:%x\n",ARP.origenMAC[0],ARP.origenMAC[1],ARP.origenMAC[2],ARP.origenMAC[3],ARP.origenMAC[4],ARP.origenMAC[5]);
	    printf("\tIP ORIGEN:  %x.%x.%x.%x\n",ARP.origenIP[0],ARP.origenIP[1],ARP.origenIP[2],ARP.origenIP[3]);
		printf("\tMAC DES: %2x:%2x:%2x:%2x:%2x:%2x\n",ARP.destinoMAC[0],ARP.destinoMAC[1],ARP.destinoMAC[2],ARP.destinoMAC[3],ARP.destinoMAC[4],ARP.destinoMAC[5]);
	    printf("\tIP DES:  %2x.%2x.%2x.%2x\n",ARP.destinoIP[0],ARP.destinoIP[1],ARP.destinoIP[2],ARP.destinoIP[3]);
	    printf("\tDestino Ethernet: %2x:%2x:%2x:%2x:%2x:%2x\n",ARP.destinoEthernet[0],ARP.destinoEthernet[1],ARP.destinoEthernet[2],ARP.destinoEthernet[3],ARP.destinoEthernet[4],ARP.destinoEthernet[5]);
	    printf("\tOrigen Ethernet: %2x:%2x:%2x:%2x:%2x:%2x\n",ARP.origenEthernet[0],ARP.origenEthernet[1],ARP.origenEthernet[2],ARP.origenEthernet[3],ARP.origenEthernet[4],ARP.origenEthernet[5]);
	    printf("\tTipo Ethernet %x\n",ARP.tipoEthernet);
		*/

	   // int cont=0;
			if(sendto(conexion,&ARP,sizeof(ARP),0,(struct sockaddr *)&saddr,sizeof(saddr))<0){
				perror("Error al envío\n");
					close(conexion);
				exit(1);
			}

			
		        bzero(&ARP, sizeof(ARP));
		        bzero((struct sockaddr*)&saddr, sizeof(saddr));
		        int n =  sizeof(saddr);
		        int flag = 0; 
		        while(flag==0){
		        if (recvfrom(conexion, &ARP, sizeof(ARP), 0, (struct sockaddr*)&ARP,&n) < 0) {
		            perror("Error recibir mensaje");
		            exit(1);
		        }
		        if ((ntohs(ARP.tipoMensaje) == ARPOP_REPLY) && !strncmp(ip,ARP.origenIP,4)) {
		           printf("*** RESPUESTA ARP ***\n");
		           printf("\t IP:  %d.%d.%d.%d\n",(int)ARP.origenIP[0],(int)ARP.origenIP[1],(int)ARP.origenIP[2],(int)ARP.origenIP[3]);
		           printf("\t MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",ARP.origenMAC[0],ARP.origenMAC[1],ARP.origenMAC[2],ARP.origenMAC[3],ARP.origenMAC[4],ARP.origenMAC[5] );
		    	flag = 1;
		        }
		    }
	 }
	close(conexion);
}