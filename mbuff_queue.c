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
#include "mbuff.h"
#include <stdio.h>

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
    if (dst->tail == NULL && dst->head == NULL) 
	dst->head = src->head;
    else 
	dst->tail->m_next = src->head;
    dst->tail	  = src->tail;
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
    if (dst->tail == NULL && dst->head == NULL)
	dst->head = src->head;
    else
	dst->tail->p_next = src->head;
    dst->tail = src->tail;
    dst->size += src->size;
    return;
}
