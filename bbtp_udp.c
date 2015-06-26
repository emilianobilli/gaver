#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>




int ipv4_udp_socket (const char *ipv4_addr, u_int16_t port)
{
    struct sockaddr_in addr;
    int udp_socket;

    addr.sin_port   = htons(port);
    addr.sin_family = AF_INET;
    inet_aton(ipv4_addr, &(addr.sin_addr));

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if ( udp_socket == -1 )
	return -1;

    if ( bind(udp_socket, (struct sockaddr *)&addr, (socklen_t)sizeof(struct sockaddr_in)) == -1 )
	return -1;

    return udp_socket;
}


int set_nofrag(int sd)
{
    int val = IP_PMTUDISC_DO;
    return setsockopt(sd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(int));
}


