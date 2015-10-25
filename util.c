
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
#define _UTIL_CODE
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <endian.h>
#include "util.h"
#define NSEC_IN_SEC 1000000000

/*======================================================================================*
 * distance(): d(a,b) -> | a - b | = | b - a |   					*
 *======================================================================================*/ 
double distance(double a, double b)
{
    double ret = a - b;
    if (ret < 0)
	return -ret
    else
	return ret;
}

/*======================================================================================*
 * htonll(): 	 									*
 *======================================================================================*/ 
u_int64_t hton64(u_int64_t u64)
{
    return htobe64(u64);
}
/*======================================================================================*
 * ntohll(): 			 							*
 *======================================================================================*/ 
u_int64_t ntoh64(u_int64_t u64)
{
    return be64toh(u64);
}

/*======================================================================================*
 * ttod(): Timespec to Double	 							*
 *======================================================================================*/ 
double ttod (struct timespec *t)
{
    return (double) ( (double) t->tv_sec ) + ( (double) t->tv_nsec / (double) NSEC_IN_SEC );
}

/*======================================================================================*
 * dtot(): Double to Timespec	 							*
 *======================================================================================*/ 
void dtot (const double *dt, struct timespec *t)
{
    t->tv_sec = floor(*dt);
    t->tv_nsec = ( *dt - (floor(*dt)) ) * (NSEC_IN_SEC) ;    /* X = X - [X] */
    return;
}

/*======================================================================================*
 * clock_monotonic()		 							*
 *======================================================================================*/ 
void clock_monotonic(struct timespec *ts)
{
    clock_gettime(CLOCK_MONOTONIC, ts);
}


/*======================================================================================*
 * timestamp_tnbo()		 							*
 *======================================================================================*/ 
void timestamp_tnbo (u_int64_t *vts, struct timespec *ts)
{
    struct timespec sts;
    if ( ts == NULL )
    {
	clock_monotonic(&sts);
	ts = &sts;
    }
    vts[0] = hton64(ts->tv_sec);
    vts[1] = hton64(ts->tv_nsec);
    return;
}

/*======================================================================================*
 * gettimestamp_fnbo()			 						*
 *======================================================================================*/ 
void gettimestamp_fnbo (struct timespec *ts, u_int64_t *vts)
{
    ts->tv_sec  = ntoh64(vts[0]);
    ts->tv_nsec = ntoh64(vts[1]);
}


/*======================================================================================*
 * timestamp()			 							*
 *======================================================================================*/ 
void timestamp (u_int64_t *vts, struct timespec *ts)
{
    struct timespec sts;
    if ( ts == NULL )
    {
	clock_monotonic(&sts);
	ts = &sts;
    }
    vts[0] = ts->tv_sec;
    vts[1] = ts->tv_nsec;
    return;
}

/*======================================================================================*
 * gettimestamp()			 							*
 *======================================================================================*/ 
void gettimestamp (struct timespec *ts, u_int64_t *vts)
{
    ts->tv_sec  = vts[0];
    ts->tv_nsec = vts[1];
}

/*======================================================================================*
 * Return the packets per seconds 							*
 *======================================================================================*/ 
long pksec (long bandwith, int mtu)
{
    double bw = (double) bandwith;
    return (long) round (bw /(double) 8 /(double) mtu);
}


/*======================================================================================*
 * Return the necesaty time in ns between one packet and other to keep the speed	*
 *======================================================================================*/
long pktime (long bandwith, int mtu)
{
    return NSEC_IN_SEC / pksec(bandwith, mtu);
}

/*======================================================================================*
 * event_timer()									*
 *======================================================================================*/
int event_timer(struct timespec *time)
{	
    int fd;
    struct itimerspec iti;

    fd = timerfd_create(CLOCK_MONOTONIC, 0);

    if (fd == -1)
	/*
         * Panic
         */
	return -1;
    iti.it_interval.tv_sec	= time->tv_sec;
    iti.it_interval.tv_nsec	= time->tv_nsec;
    iti.it_value.tv_sec		= time->tv_sec;
    iti.it_value.tv_nsec	= time->tv_nsec;

    if (timerfd_settime(fd, 0, &iti, NULL) == -1) 
    {
	/*
	 * Panic
	 */
	close(fd);
	return -1;
    }
    return fd;
}

/*======================================================================================*
 * gettimerexp()									*
 *======================================================================================*/
int gettimerexp(int fd, u_int64_t *exp)
{
    int ret;
    while (1) {
	ret = read(fd, exp, sizeof(u_int64_t));
        if (ret != -1)
	    break;
	else if (errno != EINTR)
	    break;
    }
    return ret;
}


/*======================================================================================*
 * PANIC()										*
 *======================================================================================*/
void PANIC(int err, const char *layer, const char *where)
{	
    char error[512];	
    strerror_r(err, error, 512);
    fprintf(stderr, "! ! ! GaVer panic [%s] in %s: [errno = %d] %s (EXIT) ! ! !\n", layer, where, err, error);
/*    exit(EXIT_FAILURE); */
}


