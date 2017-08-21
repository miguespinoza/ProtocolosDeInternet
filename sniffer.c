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

	strncpy(ethreq.ifr.name,"eno1",FNAMSIZ);

	ioctl(sock,SIOCGIFFLAGS,&ethreq);

	ethreq.ifr_flags |=IFF_PROMISC;

	ioctl(sock,SIOCSIFFLAGS,&ethreq);

	recvfrom(sock,buff,sizeof(buff),0,&saddr);
	printf("%s\n",buff );
	return 0;
}