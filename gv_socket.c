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
#include "gv_err.h"
#include "apitypes.h"
#include "sockopt.h"
#include <stdlib.h>


int gv_fileno(gv_socket_t *sd)
{
    if (sd->status == SO_STATUS_LINKED)
	return sd->so_data;
    /*
     * Setear el error
     */
    return -1;
}


static int getgvsocketunix(char *su_path);


static int getgvsockettype


static int getgvsocketinet(struct sockaddr_in *addr)


static int getgvsocketunix(struct sockaddr_un *addr)
{
    char *gv_socket_env;
    gv_socket_env = getenv("GV_SOCKET_LOCAL");
    if ( gv_socket_env != NULL ) {
	strcpy(addr->sun_path, gv_socket_env);
	return 0;
    }
    return -1;
}

#define EENVAR    0x01		/* Error al traer la variable de entorno */
#define ECTRLCNT  0x02		/* Error al conectarse con GaVer         */
#define EDATASO   0x03		/* Error al crear el socket de datos     */

int gv_socket(gv_socket_t *sd, int domain, int type, int protocol)
{
    char so_data_path[SU_PATH_SIZE];
    char so_ctrl_path[SU_PATH_SIZE];
    int ret;

    /*
     * Clean the struct 
     */

    memset(sd, 0, sizeof(gv_socket_t));
    /*
     * Get the Unix socket of GaVer kernel
     */
    ret = getgvsocketunix(so_ctrl_path);
    if (ret == -1)
    {
	sd->so_status = SO_STATUS_UNLINKED;
	gv_err = EENVAR;
	return -1;
    }
    
    /*
     * Connect to GaVer Kernel
     */
    sd->so_ctrl = unix_socket_client(so_ctrl_path);
    if (sd->so_ctrl == -1)
    {
	sd->so_status = SO_STATUS_UNLINKED;
	gv_err = ECTRLCNT;
	return -1;
    }

    /*
     * FALTA HACER EL HANDSHAKE --------------
     */
    tmpnam(so_data_path);
    sd->so_data = unix_socket(so_data_path);
    if (sd->so_data == -1) 
    {
	gv_err = EDATASO;
	close(sd->so_ctrl);
	sd->so_status = SO_STATUS_UNLINKED;
	return -1;
    }
    strcpy(sd->so_addr.sun_path, so_data_path);

    sd->so_status = SO_STATUS_LINKED;	
    return 0;
}


