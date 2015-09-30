/*
    This file is part of GaVer

    GaVer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GaVer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GaVer.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GV_SOCKET_H
#define GV_SOCKET_H

#define GV_DATA_SD((x)) (x)->so_data
#define GV_CTRL_SD((x)) (x)->so_ctrl

#include <sys/types.h>

#define SO_STATUS_LINKED	0
#define SO_STATUS_UNLINKED	1

typedef struct {
    int so_ctrl;			/* Control Conexion */
    int so_data;			/* Data conexion    */
    int so_status;			/* Socket Status    */
    struct sockaddr_un so_addr;		/* Unix Path 	    */
    struct sockaddr_gv local;		/* Local address    */
    struct sockaddr_gv peer;		/* Peer  address    */
} gv_socket_t;


typedef u_int16_t gv_port_t;

struct sockaddr_gv {
    sa_family_t    sin_family; 		/* address family: AF_INET */
    in_port_t      sin_port;   		/* port in network byte order */
    gv_port_t	   sin_gvport; 		/* port for gaver */
    struct in_addr sin_addr;   		/* internet address */
};



#ifndef __GV_SOCKET_API_CODE

extern int gv_socket (gv_socket_t *sd, int domain, int type, int protocol);
extern int gv_connect(gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t len);
extern int gv_accept (gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t len);
extern int gv_bind   (gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t len);
extern int gv_listen (gv_socket_t *sd, int backlog);
extern int gv_close  (gv_socket_t *sd);

extern int gv_fileno(gv_socket_t *sd);

/*
extern int gv_getsockopt()
extern int gv_setsockopt()
*/

#endif
