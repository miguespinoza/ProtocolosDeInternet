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

int main(int argc, char const *argv[])
{
	char buff[2000];

	struct sockaddr_in source,dest;
	struct sockaddr saddr;
	struct ifreq ethreq;

	int sock= socket(PF_PACKET,SOCK_RAW,ETH_P_ALL);
	if(sock)
		printf("socket bien sock: %d\n",sock);

	strncpy(ethreq.ifr_name,"eno1",IFNAMSIZ);

	printf("ethreq.ifr_name: %s\n",ethreq.ifr_name );

	int io=ioctl(sock,SIOCGIFFLAGS,&ethreq);
	printf("iocl: %d\n",io);

	ethreq.ifr_flags |=IFF_PROMISC;


	io=ioctl(sock,SIOCSIFFLAGS,&ethreq);
	printf("ioScl: %d\n",io);

	int addr_size=sizeof(saddr);
printf("addrsize %d  buffersize: %d\n",addr_size,sizeof(buff));
	int len=recvfrom(sock,&buff,sizeof(buff),0,&saddr,&addr_size);
	printf("%d\n",len );
	printf("%s\n",buff );
	return 0;
}
