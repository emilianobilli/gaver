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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "sock.h"
#include "mbuff.h"
#define _TIMERS_CODE
#include "timers.h"

static struct etqueue {
    struct exptimer *head;
    struct exptimer *tail;
    size_t size;
} etq;


void et_insert(struct exptimer *et,struct etqueue *q)
{
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * et_dequeue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct exptimer *et_dequeue(struct etqueue *q)
{
    struct exptimer *et;
    
    et = q->head;
    if (et != NULL) {
	q->head = et->et_next;
	if (q->head == NULL)
	    q->tail = NULL;
	q->size--;
	et->et_next = NULL;
    }
    return et;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_enqueue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void et_enqueue (struct etqueue *q, struct exptimer *et)
{
    et->et_next = NULL;
    if (q->tail == NULL && q->head == NULL)
        q->head = et;
    else
        q->tail->et_next = et;	
    q->tail = et;
    q->size++;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * register_et(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int  register_et (struct sock *sk, struct mbuff *mb, struct timespec *when)
{
    struct exptimer *et;
    struct timespec now;
    double dnow;
    double dwhen;

    et = calloc(sizeof(struct timespec),1);
    if (et) {
	et->et_sk    = sk;
	et->et_mb    = mb;
	et->attempts = 0;
	clock_monotonic(&now);
	dnow  = ttod(&now);
	dwhen = ttod(when);
	dnow  = dnow + dwhen;
	dtot(&dnow, &(et->et_et));
	et_insert(et,&etq);
	return 0;
    }
    return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * refresh_et(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void refresh_et (struct exptimer *et, struct timespec *when)
{
    double dw;
    double det;

    et->attempts++;
    
    dw  = ttod(when);
    det = ttod(&(et->et_et));    

    det = det + dw;
    dtot(&det,&(et->et_et));

    et_insert(et,&etq);
}	


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_etq(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_etq    (struct etqueue *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_et(): 										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_et     (void)
{
    init_etq(&etq);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * free_et(): 									    	    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void free_et (struct exptimer *et)
{
    free(et);
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * get_expired(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct exptimer *get_expired  (struct timespec *now)
{
    struct timespec ts;
    double dnow;
    double det;
    
    if (!now) {
	clock_monotonic(&ts);
	now = &ts;
    }
    dnow = ttod(now);			/* Actual Time */
    det  = ttod(&etq.head->et_et);	/* The node that expire first */
            
    if (dnow > det)
	return et_dequeue(&etq);
    return NULL;
}