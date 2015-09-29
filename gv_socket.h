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
#define __GV_SOCKET_API_CODE
#include "gv_socket.h"
#include "apitypes.h"
#include "sockopt.h"
#include <stdlib.h>

static int getgvsocketunix(char *su_path);



static int getgvsocketunix(char *su_path)
{
    char *gv_socket_env;
    gv_socket_env = getenv("GV_SOCKET_LOCAL");
    if ( gv_socket_env != NULL ) {
	strcpy(su_path, gv_socket_env);
	return 0;
    }
    return -1;
}


int gv_socket(gv_socket_t *sd, int domain, int type, int protocol);



int gv_socket(gv_socket_t *sd, int domain, int type, int protocol)
{
    char gv_server_path[SU_PATH_SIZE];
    char gv_local_path[SU_PATH_SIZE];

    if (sd == NULL)
    {

	return -1;
    }

    if (domain != AF_GAVER)
    {


    }

    if (type != SOCK_STREAM)

    ret = getgvsocketunix(gv_server_path);
    if (ret == -1)
    {
	return -1;
    }
    memset(sd, 0, sizeof(gv_socket_t));
    sd->so_ctrl = unix_socket_client(gv_server_path);
    if (sd->so_ctrl == -1)
    {
	return -1;
    }
    if (!tmpnam(gv_local_path))
    {
	return -1;
    }
    sd->so_data = unix_socket(gv_local_path);
    if (sd->so_data == -1)
    {
	return -1;
    }



    if (sd != NULL)
    {
	if (domain != AF_GAVER)
	    return -1;	/* EGVDOMAIN */

	if (type   != SOCK_STREAM)
	    return -1;  /* EGVTYPE   */
    
	if (protocol != GV_PROTOCOL)
	    return -1;  /* EGVPROTO  */ 
    
	if (getgvsocketunix(gv_server_path) == -1)
	    return -1;  /* EGVPATH   */

	memset(sd, 0, sizeof(gv_socket_t));

	if (tmpnam(gv_local_path) == NULL)
	    return -1;  /* errno */

	sd->so_ctrl = unix_socket_client(gv_server_path);
	if (sd->so_ctrl == -1)
	    return -1;  /* EGVSERVER && check errno */

	sd->so_data = unix_socket(gv_local_path);
	if (sd->so_data == -1)
	    return -1;  /* Check errno */
	
	strcpy(sd->so_addr.sun_path, gv_local_path);
	
	return 0;
    }
    return -1;
}


int getdatasocket(socket_t *sd);

int getctrlsocket(socket_t *sd);

int getgvsocketunix(char *su_path);


#endif /* gv_lib.h */
