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
#ifndef _GLO_H
#define _GLO_H

#include "types.h"
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

#include "heap_var.h"
#include "itc_var.h"
#include "output_var.h"
#include "input_var.h"
#include "start_var.h"
#include "configk_var.h"
#include "ver.h"

EXTERN int mtu;			/* MTU Max 						*/
EXTERN int ifudp;		/* Socket UDP 						*/
EXTERN int api_socket;		/* Socket For the Api 					*/
EXTERN int output_timer;	/* Timer for outgoing packages 				*/
EXTERN int refresh_timer;	/* Timer for refresh tokens 				*/
EXTERN u_int32_t local_addr;	/* Gaver Local Addres					*/
EXTERN u_int16_t local_port;	/* Gaver Local Port					*/
EXTERN u_int64_t overal_bps;	/* Max Overal Speed measured in bits per seconds	*/
EXTERN u_int64_t socket_bps;	/* Max Socket Speed measured in bits per seconds	*/
EXTERN u_int64_t input_wait;	/* Time in nsec for waiting other message		*/

/* !!!!!!!!!!!! Ojo que no se esta cargando la configuracion en las variables globales */


#endif /* _GLO_H */