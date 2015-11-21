
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
#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"
#include <time.h>
#include <sys/types.h>

#ifdef _UTIL_CODE
#undef EXTERN
#define EXTERN
#endif




/*
 * Macros to conver from gbps, mbps and kbsp to bps
 */

#define GBPS_TOBPS(x) ((x) * 1000000000)
#define MBPS_TOBPS(x) ((x) * 1000000)
#define KBPS_TOBPS(x) ((x) * 1000)

#define PACKETS_PER_ROUND 100

#define TS_SEC  0
#define TS_NSEC 1

/*======================================================================================*
 * clock_monotonic()		 							*
 *======================================================================================*/ 
EXTERN void clock_monotonic(struct timespec *ts);

/*======================================================================================*
 * htonll():			 							*
 *======================================================================================*/ 
EXTERN u_int64_t hton64 (u_int64_t);

/*======================================================================================*
 * pfloat():			 							*
 *======================================================================================*/  
EXTERN double pfloat(double value);

/*======================================================================================*
 * ntohll():			 							*
 *======================================================================================*/  
EXTERN u_int64_t ntoh64 (u_int64_t);

/*======================================================================================*
 * ttod(): Timespec to Double	 							*
 *======================================================================================*/ 
EXTERN double ttod (struct timespec *t);

/*======================================================================================*
 * ttod2(): Timespec to Double	 							*
 *======================================================================================*/ 
EXTERN double ttod2 (struct timespec *t);


/*======================================================================================*
 * dtot(): Double to Timespec	 							*
 *======================================================================================*/ 
EXTERN void dtot (const double *dt, struct timespec *t);

/*======================================================================================*
 * getts()			 							*
 *======================================================================================*/ 
EXTERN void gettimestamp (struct timespec *ts, u_int64_t *vts);

/*======================================================================================*
 * timestamp()			 							*
 *======================================================================================*/ 
EXTERN void timestamp (u_int64_t *vts, struct timespec *ts);


/*======================================================================================*
 * getts()			 							*
 *======================================================================================*/ 
EXTERN void gettimestamp_fnbo (struct timespec *ts, u_int64_t *vts);

/*======================================================================================*
 * timestamp()			 							*
 *======================================================================================*/ 
EXTERN void timestamp_tnbo (u_int64_t *vts, struct timespec *ts);


/*======================================================================================*
 * Return packets per second based in overal speed					*
 *======================================================================================*/
EXTERN double getpksec (u_int64_t speed_bps, int mtu);

/*======================================================================================*
 * Return the necesary time to send PACKETS_PER_ROUND					*
 *======================================================================================*/
EXTERN double getreftime (u_int64_t speed_bps, int mtu);

/*======================================================================================*
 * Return the numbers of packages available to send for each refresh_time		*
 *======================================================================================*/
EXTERN double getpkrtime (u_int64_t speed_bps, int mtu, double refresh_time);

/*======================================================================================*
 * Return the packets per seconds 							*
 *======================================================================================*/ 
EXTERN long pksec (long bandwith, int mtu);

/*======================================================================================*
 * Return the necesaty time in ns between one packet and other to keep the speed	*
 *======================================================================================*/
EXTERN long pktime (u_int64_t bandwith, int mtu);

/*======================================================================================*
 * event_timer()									*
 *======================================================================================*/
EXTERN int event_timer(struct timespec *time);

/*======================================================================================*
 * gettimerexp()									*
 *======================================================================================*/
EXTERN int gettimerexp(int fd, u_int64_t *exp);

/*======================================================================================*
 * realspeed(): Return the real clock speed 						*
 *======================================================================================*/ 
EXTERN u_int64_t realspeed(u_int64_t speed);


/*======================================================================================*
 * update_token()									*
 * 	avtok: Tokens disponibles, debe ser un valor entre 0 y 1 -> [0;1)		*
 * 	reftok: Cantidad de tokens a actualizar, de acuerdo a la velocidad -> (0;MAX]	*
 *	cwrp: Porcentaje disponible a utilizar del refresh, este valor se ajusto de 	*
 *		acuerdo a la congestion actual, toma un valor entre [0;100]		*
 *	rwinav: Ventana disponible para emision, este parametro deberia ser igual a 	*
 *		la ventana anunciada por el receptor - la cantidad de paquetes en vuelo *
 *======================================================================================*/
EXTERN double update_token (double avtok, double reftok, u_int8_t cwrp, double rwinav);


/*======================================================================================*
 * PANIC()										*
 *======================================================================================*/
EXTERN void PANIC(int err, const char *layer, const char *where);

#endif