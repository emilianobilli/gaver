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
#define _HEAP_CODE
#include "heap.h"
#include <string.h>

static void init_heap_mbuff(size_t len);
static void init_heap_msg(size_t len);


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
 * clone_mbuff()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct mbuff *clone_mbuff(struct mbuff *mb)
{
    struct mbuff *ret;

    ret = alloc_mbuff_locking();
    if (ret != NULL) {
	memcpy(ret, mb, sizeof(struct mbuff));
	ret->m_next = NULL;
    }
	
    return ret;
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
 * alloc_mbuff_queue()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t alloc_mbuff_queue (struct mb_queue *queue, size_t len)
{
    struct mbuff *mb;
    size_t i;

    init_mbuff_queue(queue);

    if (!len)
	return 0;

    for (i = 0; i < len; i++) {
	mb = alloc_mbuff();
	if (mb) 
	    mbuff_enqueue(queue, mb);
	else
	    break;
    }
    return i;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_msg_queue()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t alloc_msg_queue (struct msg_queue *queue, size_t len)
{
    struct msg *mp;
    size_t i;

    init_msg_queue(queue);

    if (!len)
	return 0;

    for (i = 0; i < len; i++) {
	mp = alloc_msg();
	if (mp) 
	    msg_enqueue(queue, mp);
	else
	    break;
    }
    return i;
}




/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_msg_chain()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t alloc_msg_chain( struct msg_queue *queue, size_t len)
{
    struct msg *p_buff;
    size_t i;

    pthread_mutex_lock(&heap_msg_mutex);

    for ( i = 0; i <= len -1; i++ ) {
	p_buff = alloc_msg();
	if ( p_buff != NULL ) {
	    p_buff->p_next = NULL;
	    p_buff->mb.mbp = NULL;
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
size_t alloc_mbuff_payload ( struct mb_queue *queue, size_t len, size_t payload_size )
{
    size_t mbuff_elements = len / payload_size;
    size_t ret;

    pthread_mutex_lock(&heap_mbuff_mutex);

    ret = alloc_mbuff_queue(queue, mbuff_elements);

    pthread_mutex_unlock(&heap_mbuff_mutex);

    return ret;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_mbuff_chain()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t alloc_mbuff_chain( struct msg_queue *queue, size_t len)
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
	        p_buff->mb.mbp = m_buff;
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
void init_heap_mbuff(size_t len)
{
    alloc_mbuff_queue(&heap_mbuff, len);
    pthread_mutex_init(&heap_mbuff_mutex,NULL);

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap_msg()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap_msg(size_t len)
{
    struct msg_queue q;
    
    init_msg_queue(&heap_msg);
    alloc_msg_queue(&q, len);
    msgmove(&heap_msg,&q);
    pthread_mutex_init(&heap_msg_mutex,NULL);

    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_heap()										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_heap(size_t len)
{
    heap_mem = 0;
    init_heap_mbuff(len);
    init_heap_msg(len);

    return;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * heap_mbuff_size()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t heap_mbuff_size(void)
{
    return heap_mbuff.size;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * heap_msg_size()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t heap_msg_size(void)
{
    return heap_msg.size;
}
