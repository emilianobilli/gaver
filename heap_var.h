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

#ifndef _HEAP_VAR_H
#define _HEAP_VAR_H


#include "types.h"
#include "mbuff.h"
#include <pthread.h>
#include <stdlib.h>

EXTERN size_t heap_mem;
EXTERN struct mb_queue  heap_mbuff;
EXTERN pthread_mutex_t  heap_mbuff_mutex;

EXTERN struct pkt_queue heap_pkt;
EXTERN pthread_mutex_t  heap_pkt_mutex;

#endif /* _HEAP_VAR_H */


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
#ifndef _HEAP_UTIL_H
#define _HEAP_UTIL_H
#include "heap_var.h"

EXTERN void free_mbuff(struct mbuff *m);
EXTERN void free_mbuff_locking(struct mbuff *m);

EXTERN void free_pkt(struct pktbuff *p);
EXTERN void free_pkt_locking(struct pktbuff *p);

EXTERN struct mbuff *alloc_mbuff(void);
EXTERN struct mbuff *alloc_mbuff_locking(void);
EXTERN int alloc_mbuff_chain( struct pktqueue *queue, size_t len);

EXTERN struct pktbuff *alloc_pkt(void);
EXTERN struct pktbuff *alloc_pkt_locking(void);

EXTERN void init_heap_mbuff(void);
EXTERN void init_heap_pkt(void);
EXTERN void init_heap(void);

#endif

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



void free_mbuff(struct mbuff *m)
void free_mbuff_locking(struct mbuff *m)
{
    pthread_mutex_lock(&heap_mbuff_mutex);
    free_mbuff(m);
    pthread_mutex_unlock(&heap_mbuff_mutex);
    return;
}

void free_pkt(struct pktbuff *p)
void free_pkt_locking(struct pktbuff *p)
{
    pthread_mutex_lock(&heap_pkt_mutex);
    free_pkt(p);
    pthread_mutex_unlock(&heap_pkt_mutex);
    return;
}


struct mbuff *alloc_mbuff(void)
struct mbuff *alloc_mbuff_locking(void)
{
    struct mbuff *ret;

    pthread_mutex_lock(&heap_mbuff_mutex);

    ret = alloc_mbuff();

    pthread_mutex_unlock(&heap_mbuff_mutex);

    return ret;
}


int alloc_mbuff_chain( struct pktqueue *queue, size_t len)
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
    /*
     * 1, 2
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
    /*
     * 5, 6
     */
    pthread_mutex_unlock(&heap_mbuff_mutex);
    pthread_mutex_unlock(&heap_pkt_mutex);

    return i;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct pktbuff *alloc_pkt(void)
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

void init_heap_mbuff(void)
{
    heap_mbuff.head = NULL;
    heap_mbuff.tail = NULL;
    heap_mbuff_mutex = PTHREAD_MUTEX_INITIALIZER;

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void init_heap_pkt(void)
{
    heap_pkt.head = NULL;
    heap_pkt.tail = NULL;
    heap_pkt_mutex = PTHREAD_MUTEX_INITIALIZER;

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




