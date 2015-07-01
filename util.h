
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


/*======================================================================================*
 * Return the packets per seconds 							*
 *======================================================================================*/ 
EXTERN long pksec (long bandwith, int mtu);

/*======================================================================================*
 * Return the necesaty time in ns between one packet and other to keep the speed	*
 *======================================================================================*/
EXTERN long pktime (long bandwith, int mtu);

/*======================================================================================*
 * event_timer()									*
 *======================================================================================*/
EXTERN int event_timer(struct timespec *time);

/*======================================================================================*
 * gettimerexp()									*
 *======================================================================================*/
EXTERN int gettimerexp(int fd, u_int64_t *exp);

/*======================================================================================*
 * PANIC()										*
 *======================================================================================*/
EXTERN void PANIC(int err, const char *layer, const char *where);

#endif