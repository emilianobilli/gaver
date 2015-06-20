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
#include <stdio.h>
#include <stdlib.h>
#include "heap_var.h"
#include "mbuff_queue.h"

static void init_heap_mbuff(void);
static void init_heap_pkt(void);



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct pktbuff *alloc_pkt(void)
{
    struct pktbuff *pkt;

    if (heap_pktbuff.head == NULL && heap_pktbuff.tail == NULL) {
	pkt = (struct pktbuff *)calloc(1, sizeof(struct pktbuff));
	heap_mem += (pkt == NULL) ? 0 : sizeof(struct pktbuff);
    }
    else
	pkt = pktbuff_dequeue(&heap_pktbuff);

    return pkt;
}

struct pktbuff *alloc_pkt_locking(void)
{
    struct pktbuff *ret;

    pthread_mutex_lock(&heap_pkt_mutex);

    ret = alloc_pkt();

    pthread_mutex_unlock(&heap_pkt_mutex);
    return ret;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


void free_mbuff(struct mbuff *m)
{
    mbuff_enqueue(&heap_mbuff, m);	
    return;
}

void free_mbuff_locking(struct mbuff *m)
{
    pthread_mutex_lock(&heap_mbuff_mutex);
    free_mbuff(m);
    pthread_mutex_unlock(&heap_mbuff_mutex);
    return;
}

void free_pkt(struct pktbuff *p)
{
    pktbuff_enqueue(&heap_pktbuff, p);
    return;
}

void free_pkt_locking(struct pktbuff *p)
{
    pthread_mutex_lock(&heap_pkt_mutex);
    free_pkt(p);
    pthread_mutex_unlock(&heap_pkt_mutex);
    return;
}


struct mbuff *alloc_mbuff(void)
{
    struct mbuff *mb;
    if (heap_mbuff.head == NULL && heap_mbuff.tail == NULL) {
	mb = (struct mbuff *) calloc(1, sizeof(struct mbuff));
	heap_mem += (mb == NULL) ? 0 : sizeof(struct mbuff);
    }
    else
	mb = mbuff_dequeue(&heap_mbuff);

    return mb;
}

struct mbuff *alloc_mbuff_locking(void)
{
    struct mbuff *ret;

    pthread_mutex_lock(&heap_mbuff_mutex);

    /* Critical Region */
    ret = alloc_mbuff();

    pthread_mutex_unlock(&heap_mbuff_mutex);

    return ret;
}


int alloc_mbuff_chain( struct pkt_queue *queue, size_t len)
{
    struct pktbuff *p_buff;
    struct mbuff   *m_buff;
    size_t i;
    /*
     * 1- Block mutex pkt
     * 2- Block mutex mbuff
     * 3- Create n mbuff elements
     * 4- Create n pkbuff elements
     * 5- Unblock mutex mbuff
     * 6- Unblock mutex pkt
     */
    pthread_mutex_lock(&heap_pkt_mutex);
    pthread_mutex_lock(&heap_mbuff_mutex);

    for ( i = 0; i <= len-1 ; i++ ) {
	m_buff = alloc_mbuff();
	if (m_buff != NULL) {
	    p_buff = alloc_pkt();
	    if (p_buff != NULL) {
		p_buff->p_next = NULL;
	        p_buff->p_mbuff = m_buff;
		pktbuff_enqueue(queue, p_buff);
	    }
	    else {
		free_mbuff(m_buff);
		break;
	    }
	}
	else
	    break;
    }
    pthread_mutex_unlock(&heap_mbuff_mutex);
    pthread_mutex_unlock(&heap_pkt_mutex);

    return i;
}


void init_heap_mbuff(void)
{
    heap_mbuff.head = NULL;
    heap_mbuff.tail = NULL;
    pthread_mutex_init(&heap_mbuff_mutex,NULL);

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap_pkt(void)
{
    heap_pktbuff.head = NULL;
    heap_pktbuff.tail = NULL;
    pthread_mutex_init(&heap_pkt_mutex,NULL);

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap(void)
{
    heap_mem = 0;
    init_heap_mbuff();
    init_heap_pkt();

    return;
}




