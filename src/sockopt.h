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
#include "types.h"

#ifdef _SOCKOPT_CODE
#undef EXTERN
#define EXTERN
#endif

EXTERN int unix_socket(const char *path);

EXTERN int unix_socket_client(const char *path);

EXTERN int ipv4_udp_socket (const char *ipv4_addr, u_int16_t port);

EXTERN int ipv4_udp_socket_nbo (u_int32_t addr, u_int16_t port);

EXTERN int __ipv4_udp_socket (struct sockaddr_in *addr);

EXTERN int iptos_throughput(int sd);

EXTERN int set_reuseaddr(int socket);

EXTERN int set_reuseaddr_unix(const char *path);

EXTERN int set_rcvbuf(int socket, u_int32_t buff_size );

EXTERN int set_sndbuf(int socket, u_int32_t buff_size );

EXTERN int set_nofrag(int sd);

EXTERN int icmp_socket(void);

EXTERN int select_nosignal (int max, fd_set *read, fd_set *write, fd_set *excep, struct timeval *tout);

#endif