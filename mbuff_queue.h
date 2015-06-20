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
#ifndef _MBUFF_QUEUE_H
#define _MBUFF_QUEUE_H

#include "mbuff.h"
#include "types.h"


/*
 * ToDo: mbuff_insert()
 */

EXTERN void mbuff_enqueue (struct mb_queue *queue, struct mbuff *mb);

EXTERN struct mbuff *mbuff_dequeue(struct mb_queue *queue);

EXTERN void mbuffqcat (struct mb_queue *dst, struct mb_queue *src);

EXTERN void pktbuff_enqueue (struct pkt_queue *queue, struct pktbuff *pkt);

EXTERN struct pktbuff *pktbuff_dequeue(struct pkt_queue *queue);

EXTERN void pktbuffqcat (struct pkt_queue *dst, struct pkt_queue *src);

#endif