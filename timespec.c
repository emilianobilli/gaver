#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NANOSECOND 999999999


int time_gt (struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec)
	return 1;


    if (a->tv_sec < b->tv_sec)
	return 0;
    
    if (a->tv_sec == b->tv_sec)
    {
	if (a->tv_nsec > b->tv_nsec){
	    return 1;
	}
	if (a->tv_nsec <= b->tv_nsec){
	    return 0;
	}
    }
}


int time_lt (struct timespec *a, struct timespec *b)
{
    if (a->tv_sec < b->tv_sec){
	return 1;
    }

    if (a->tv_sec > b->tv_sec){
	return 0;
    }
	
    if (a->tv_sec == b->tv_sec){
	if (a->tv_nsec < b->tv_nsec){
	    return 1;
	}
	if (a->tv_nsec >= b->tv_nsec){
	    return 0;
	}
    }
}

int time_eq (struct timespec *a, struct timespec *b)
{
    if ((a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec))
	return 1;
    return 0;
}

void time_diff (struct timespec *a, struct timespec *b, struct timespec *res){
    if (time_gt (a, b)){
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (res->tv_nsec < 0){
	    res->tv_nsec = 1000000000 + res->tv_nsec;
	    res->tv_sec = res->tv_sec -1;
	}
    }
    else {
	res->tv_sec = b->tv_sec - a->tv_sec;
	res->tv_nsec = b->tv_nsec - a->tv_nsec;
	if (res->tv_nsec < 0){
	    res->tv_nsec = 1000000000 + res->tv_nsec;
	    res->tv_sec = res->tv_sec -1;
	}
    }
    return;

}




void time_add (struct timespec *a, struct timespec *b, struct timespec *res){
    res->tv_sec = a->tv_sec + b->tv_sec;
    res->tv_nsec = a->tv_nsec + b->tv_nsec;
    if (res->tv_nsec > MAX_NANOSECOND){
	res->tv_nsec = res->tv_nsec - (MAX_NANOSECOND + 1);
	res->tv_sec = res->tv_sec + 1;
    }
    return;
}

int timer_expiration (clockid_t clock_type, struct timespec *b, struct timespec *res){
    struct timespec now;
    
    clock_gettime (clock_type, &now);
    printf ("Ahora: sec: %ld - nsec: %ld\n", now.tv_sec, now.tv_nsec);
    time_diff (b, &now, res);
    if (time_gt(b, &now))
	return 0;
    return 1;
}



main(){
    struct timespec test, test2, diff, sum, exp;
    int res;
    
//    clock_gettime(CLOCK_MONOTONIC, &test);
    test.tv_sec = 2000000;
    test.tv_nsec = 99;
    test2.tv_sec = 40;
    test2.tv_nsec = 100;
    printf ("test\n sec: %ld\n nsec: %ld\n", test.tv_sec, test.tv_nsec);
    printf ("test2\n sec: %ld\n nsec: %ld\n", test2.tv_sec, test2.tv_nsec);
    res = time_gt(&test, &test2);
    printf ("resultado mayor: %d\n", res);
    res = time_lt(&test, &test2);
    printf ("resultado menor: %d\n", res);
    res = time_eq(&test, &test2);
    printf ("resultado igual: %d\n", res);
    time_diff (&test, &test2, &diff);
    printf ("resultado diff: sec: %ld - nsec: %ld\n", diff.tv_sec, diff.tv_nsec);
    time_add (&test2, &diff, &sum);
    printf ("resultado suma: sec: %ld - nsec: %ld\n", sum.tv_sec, sum.tv_nsec);
    res = timer_expiration(CLOCK_MONOTONIC, &test, &exp);
    printf ("expiration: %d - sec: %ld - nsec: %ld\n", res, exp.tv_sec, exp.tv_nsec);
}
