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
static void init_heap_msg(void);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_msg(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct msg *alloc_msg(void)
{
    struct msg *msg;

    if (heap_msg.head == NULL && heap_msg.tail == NULL) {
	msg = (struct msg *)calloc(1, sizeof(struct msg));
	heap_mem += (msg == NULL) ? 0 : sizeof(struct msg);
    }
    else
	msg = msg_dequeue(&heap_msg);

    return msg;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_msg_locking():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct msg *alloc_msg_locking(void)
{
    struct msg *ret;

    pthread_mutex_lock(&heap_msg_mutex);

    ret = alloc_msg();

    pthread_mutex_unlock(&heap_msg_mutex);
    return ret;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * free_mbuff()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void free_mbuff(struct mbuff *m)
{
    /*
     * Falta borrar estructura
     */

    mbuff_enqueue(&heap_mbuff, m);	
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * free_mbuff_locking()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void free_mbuff_locking(struct mbuff *m)
{
    pthread_mutex_lock(&heap_mbuff_mutex);
    free_mbuff(m);
    pthread_mutex_unlock(&heap_mbuff_mutex);
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * free_msg()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void free_msg(struct msg *p)
{
    /*
     * Falta borrar estructura
     */

    msg_enqueue(&heap_msg, p);
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * free_msg()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void free_msg_locking(struct msg *p)
{
    pthread_mutex_lock(&heap_msg_mutex);
    free_msg(p);
    pthread_mutex_unlock(&heap_msg_mutex);
    return;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_mbuff()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
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

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_mbuff_locking()								    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct mbuff *alloc_mbuff_locking(void)
{
    struct mbuff *ret;

    pthread_mutex_lock(&heap_mbuff_mutex);

    /* Critical Region */
    ret = alloc_mbuff();

    pthread_mutex_unlock(&heap_mbuff_mutex);

    return ret;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_msg_chain()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int alloc_msg_chain( struct msg_queue *queue, size_t len)
{
    struct msg *p_buff;
    size_t i;

    pthread_mutex_lock(&heap_msg_mutex);

    for ( i = 0; i <= len -1; i++ ) {
	p_buff = alloc_msg();
	if ( p_buff != NULL ) {
	    p_buff->p_next = NULL;
	    p_buff->mb.p_mbuff = NULL;
	    msg_enqueue(queue, p_buff);
	}
	else
	    break;
    }
    
    pthread_mutex_unlock(&heap_msg_mutex);
    return i;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_mbuff_payload()								    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int alloc_mbuff_payload( struct mb_queue *queue, size_t len, int mtu )
{
    struct mbuff *mptr;
    size_t mbuff_elements = len / (size_t) mtu;
    size_t i;

    pthread_mutex_lock(&heap_mbuff_mutex);
    for ( i = 0; i <= mbuff_elements -1; i++ ) {
	mptr = alloc_mbuff();
	if (mptr == NULL)
	    break;
	mbuff_enqueue(queue, mptr);
    }
    pthread_mutex_unlock(&heap_mbuff_mutex);

    return (int) i;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_mbuff_chain()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int alloc_mbuff_chain( struct msg_queue *queue, size_t len)
{
    struct msg	 *p_buff;
    struct mbuff *m_buff;
    size_t i;
    /*
     * 1- Block mutex msg
     * 2- Block mutex mbuff
     * 3- Create n mbuff elements
     * 4- Create n pkbuff elements
     * 5- Unblock mutex mbuff
     * 6- Unblock mutex msg
     */
    pthread_mutex_lock(&heap_msg_mutex);
    pthread_mutex_lock(&heap_mbuff_mutex);

    for ( i = 0; i <= len-1 ; i++ ) {
	m_buff = alloc_mbuff();
	if (m_buff != NULL) {
	    p_buff = alloc_msg();
	    if (p_buff != NULL) {
		p_buff->p_next = NULL;
	        p_buff->mb.p_mbuff = m_buff;
		msg_enqueue(queue, p_buff);
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
    pthread_mutex_unlock(&heap_msg_mutex);

    return i;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap_mbuff()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap_mbuff(void)
{
    heap_mbuff.size = 0;
    heap_mbuff.head = NULL;
    heap_mbuff.tail = NULL;
    pthread_mutex_init(&heap_mbuff_mutex,NULL);

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap_msg()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap_msg(void)
{
    heap_msg.size = 0;
    heap_msg.head = NULL;
    heap_msg.tail = NULL;
    pthread_mutex_init(&heap_msg_mutex,NULL);

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap(void)
{
    heap_mem = 0;
    init_heap_mbuff();
    init_heap_msg();

    return;
}




