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
