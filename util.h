#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

#ifdef _UTIL_CODE
#undef EXTERN
#define EXTERN
#endif

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


#endif