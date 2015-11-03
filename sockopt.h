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

#ifndef _SOCKOPT_H
#define _SOCKOPT_H

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

extern int unix_socket(const char *path);

extern int unix_socket_client(const char *path);

extern int ipv4_udp_socket (const char *ipv4_addr, u_int16_t port);

extern int ipv4_udp_socket_nbo (u_int32_t addr, u_int16_t port);

extern int __ipv4_udp_socket (struct sockaddr_in *addr);

extern int iptos_throughput(int sd);

extern int set_rcvbuf(int socket, u_int32_t buff_size );

extern int set_sndbuf(int socket, u_int32_t buff_size );

extern int set_nofrag(int sd);

extern int select_nosignal (int max, fd_set *read, fd_set *write, fd_set *excep, struct timeval *tout);

#endif