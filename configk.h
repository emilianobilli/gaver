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
#ifndef _CONFIGK_H
#define _CONFIGK_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "types.h"


#define UNIX_PATH_MAX    108

struct configk {
    struct in_addr	addr;		/* Network bit order addr */
    u_int16_t		port;		/* Network bit order port */
    char		listen_api[UNIX_PATH_MAX];
    int			sched;
    int			mtu;
    u_int64_t		overal_bps;	/* Velocidad Maxima */	
    u_int64_t		socket_bps;	/* Velocidad Maxima por socket */
    u_int32_t		rmem;		/* Memoria Maxima por Socket para lectura   */
    u_int32_t		wmem;		/* Memoria Maxima por Socket para escritura */
};

#ifndef _CONFIGK_CODE
/*======================================================================================*
 * loadcfgk(): Load the command line configuration passed in argc and *argv[]		*
 *		   The function force to exit process when error ocurs			*
 *======================================================================================*/
EXTERN int loadcfgk (int argc, char *argv[], struct configk *cfg);
EXTERN void dumpcfgk (FILE *f, struct configk *cfg);
#endif

#endif /* configk.h */
