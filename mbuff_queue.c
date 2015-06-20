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
}

struct mbuff *mbuff_dequeue(struct mb_queue *queue)
{
    struct mbuff *mb;
    
    mb = queue->head;
    if (mb != NULL) {
	queue->head = mb->m_next;
	if (queue->head == NULL)
	    queue->tail = NULL;
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
    return;
}

void pktbuff_enqueue (struct pkt_queue *queue, struct pktbuff *pkt)
{
    pkt->p_next = NULL;
    if (queue->tail == NULL && queue->head == NULL)
	queue->head = pkt;
    else
	queue->tail->p_next = pkt;
    queue->tail = pkt;
}

struct pktbuff *pktbuff_dequeue(struct pkt_queue *queue)
{
    struct pktbuff *pkt;
    pkt = queue->head;
    if (pkt != NULL) {
	queue->head = pkt->p_next;
	if (queue->head == NULL)
	    queue->tail = NULL;
    }
    return pkt;
}
void pktbuffqcat (struct pkt_queue *dst, struct pkt_queue *src)
{
    if (dst->tail == NULL && dst->head == NULL)
	dst->head = src->head;
    else
	dst->tail->p_next = src->head;
    dst->tail = src->tail;
    return;
}
