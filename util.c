
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
#include <unistd.h>
#include "util.h"
#define NSEC_IN_SEC 1000000000L


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
	ret = read(fd, exp, 8);
        if (ret != -1)
	    break;
	else if (errno != EINTR)
	    break;
    }
    return ret;
}


