/*
    This file is part of GaVer

    GaVer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/un.h>
#include <string.h>


int unix_socket_client(const char *path)
{
    struct sockaddr_un addr;
    int unix_socket;

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_socket == -1)
	return -1;

    if (connect(unix_socket,
	       (struct sockaddr *)&addr,
	       (socklen_t) sizeof(struct sockaddr_un)) == 1)
	return -1;

    return unix_socket;
}
    

int unix_socket(const char *path)
{
    struct sockaddr_un addr;
    int unix_socket;

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_socket == -1)
	return -1;

    if (bind(unix_socket, 
	    (struct sockaddr *)&addr,
	    (socklen_t) sizeof(struct sockaddr_un)) == -1)
	return -1;
    if (listen(unix_socket,5) == -1)
	return -1;

    return unix_socket;
}

int __ipv4_udp_socket(struct sockaddr_in *addr)
{
    int udp_socket;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if ( udp_socket == -1 )
	return -1;

    if (bind(udp_socket, 
	    (struct sockaddr *)addr, 
	    (socklen_t)sizeof(struct sockaddr_in)) == -1 )
	return -1;

    return udp_socket;
}

int ipv4_udp_socket_nbo ( u_int32_t addr, u_int16_t port)
{
    struct sockaddr_in saddr;
    
    saddr.sin_port = port;
    saddr.sin_addr.s_addr = addr;
    saddr.sin_family = AF_INET;

    return __ipv4_udp_socket(&saddr);
}

int ipv4_udp_socket (const char *ipv4_addr, u_int16_t port)
{
    struct sockaddr_in addr;
    int udp_socket;

    addr.sin_port   = htons(port);
    addr.sin_family = AF_INET;
    inet_aton(ipv4_addr, &(addr.sin_addr));

    return __ipv4_udp_socket(&addr);
}


int iptos_throughput(int socket)
{
    int tos = IPTOS_THROUGHPUT;
    return setsockopt(socket, IPPROTO_IP, IP_TOS, &tos, sizeof(int));
}

int set_rcvbuf(int socket, u_int32_t buff_size )
{
    return setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(u_int32_t));
}

int set_sndbuf(int socket, u_int32_t buff_size )
{
    return setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &buff_size, sizeof(u_int32_t));
}

int set_nofrag(int sd)
{
    int val = IP_PMTUDISC_DO;
    return setsockopt(sd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(int));
}

