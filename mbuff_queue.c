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
#define _MBUFF_QUEUE_CODE
#include "mbuff.h"
#include <stdio.h>
#include "mbuff_queue.h"

void init_mbuff_queue(struct mb_queue *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void init_msg_queue(struct msg_queue *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void mbuff_enqueue (struct mb_queue *queue, struct mbuff *mb)
{
    mb->m_next = NULL;
    if (queue->tail == NULL && queue->head == NULL)
        queue->head = mb;
    else
        queue->tail->m_next = mb;	
    queue->tail = mb;
    queue->size++;
}

void mbuff_insert (struct mb_queue *queue, struct mbuff *mb)
{
    struct mbuff    *ptr;
    struct mb_queue tmp;

    if (queue->size == 0 || 
	mb->m_hdr.seq > queue->tail->m_hdr.seq)
	mbuff_enqueue(queue, mb);
    else {
	if (mb->m_hdr.seq < queue->head->m_hdr.seq) {
	    mb->m_next  = queue->head;
	    queue->head = mb;
	    queue->size++;
	}
	else {
	    init_mbuff_queue(&tmp);
	    while ((ptr = mbuff_dequeue(queue)) != NULL )
	    {
		if (ptr->m_hdr.seq < mb->m_hdr.seq)
		    mbuff_enqueue(&tmp, ptr);
		else {
		    mbuff_enqueue(&tmp, mb);
		    mbuff_enqueue(&tmp, ptr);
		    break;
		}
	    }
	    mbuffqcat(&tmp, queue);
	    queue->head = tmp.head;
	    queue->tail = tmp.tail;
	    queue->size = tmp.size;
	}
    }
}


struct mbuff *mbuff_dequeue(struct mb_queue *queue)
{
    struct mbuff *mb;
    
    mb = queue->head;
    if (mb != NULL) {
	queue->head = mb->m_next;
	if (queue->head == NULL)
	    queue->tail = NULL;
	queue->size--;
	mb->m_next = NULL;
    }
    return mb;
}

void mbuffqcat (struct mb_queue *dst, struct mb_queue *src)
{
    if (src->tail == NULL && src->tail == NULL)
	return;

    if (dst->tail == NULL && dst->head == NULL) 
	dst->head = src->head;
    else 
	dst->tail->m_next = src->head;

    dst->tail  = src->tail;
    dst->size += src->size;
    return;
}

void msg_enqueue (struct msg_queue *queue, struct msg *msg)
{
    msg->p_next = NULL;
    if (queue->tail == NULL && queue->head == NULL)
	queue->head = msg;
    else
	queue->tail->p_next = msg;
    queue->tail = msg;
    queue->size++;
}

struct msg *msg_dequeue(struct msg_queue *queue)
{
    struct msg *msg;
    msg = queue->head;
    if (msg != NULL) {
	queue->head = msg->p_next;
	if (queue->head == NULL)
	    queue->tail = NULL;
	queue->size--;
	msg->p_next = NULL;
    }
    return msg;
}
void msgqcat (struct msg_queue *dst, struct msg_queue *src)
{
    if (src->head == NULL && src->tail == NULL)
	return;

    if (dst->tail == NULL && dst->head == NULL)
	dst->head = src->head;
    else
	dst->tail->p_next = src->head;
    dst->tail = src->tail;
    dst->size += src->size;
    return;
}

size_t  msgnmove (struct msg_queue *dst, struct msg_queue *src, size_t n)
{
    size_t i;
    struct msg *mptr;

    if (n == 0)
	return 0;

    for ( i = 0; i <= n-1; i++ ) {
	mptr = msg_dequeue(src);
	if (mptr == NULL)
	    break;
	msg_enqueue(dst, mptr);
    }
    return i;
}
