#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NANOSECOND   999999999
#define NSSECOND	1000000000

int time_gt (struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec)
	return 1;
    if (a->tv_sec < b->tv_sec)
	return 0;

    if (a->tv_sec == b->tv_sec)
    {
	if (a->tv_nsec > b->tv_nsec)
	    return 1;
	else
	    return 0;
    }
}


int time_lt (struct timespec *a, struct timespec *b)
{
    if (a->tv_sec < b->tv_sec)
	return 1;

    if (a->tv_sec > b->tv_sec)
	return 0;
	
    if (a->tv_sec == b->tv_sec) 
    {
	if (a->tv_nsec < b->tv_nsec)
	    return 1;
	else
	    return 0;
    }
}

int time_eq (struct timespec *a, struct timespec *b)
{
    if ((a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec))
	return 1;
    return 0;
}

void time_diff (struct timespec *a, struct timespec *b, struct timespec *res)
{
    if (time_gt (a, b))
    {
	res->tv_sec  = a->tv_sec  - b->tv_sec;
	res->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (res->tv_nsec < 0)
	{
	    res->tv_nsec = NSSECOND + res->tv_nsec;
	    res->tv_sec = res->tv_sec -1;
	}
    }
    else 
    {
	res->tv_sec  = b->tv_sec - a->tv_sec;
	res->tv_nsec = b->tv_nsec - a->tv_nsec;
	if (res->tv_nsec < 0)
	{
	    res->tv_nsec = NSSECOND + res->tv_nsec;
	    res->tv_sec = res->tv_sec -1;
	}
    }
    return;
}

void time_add (struct timespec *a, struct timespec *b, struct timespec *res)
{
    res->tv_sec  = a->tv_sec  + b->tv_sec;
    res->tv_nsec = a->tv_nsec + b->tv_nsec;
    if (res->tv_nsec > MAX_NANOSECOND)
    {
	res->tv_nsec = res->tv_nsec - (MAX_NANOSECOND + 1);
	res->tv_sec = res->tv_sec + 1;
    }
    return;
}

int timer_exp (struct timespec *now, struct timespec *b, struct timespec *res)
{
    struct timespec now_te, *ptr_te;
    
    if ( now == NULL )
    {
	clock_gettime (CLOCK_MONOTONIC, &now_te);
	ptr_te = &now_te;
    }
    else
	ptr_te = now;

    if (res != NULL) 
	time_diff (b, ptr_te, res);

    if (time_gt(b, pte_te))
	return 0;

    return 1;
}

