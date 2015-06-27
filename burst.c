#include <math.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <errno.h>
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
int gettimerexp(int fd, u_int64_t *exp);
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


