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

#ifdef _MBUFF_QUEUE_CODE
#undef EXTERN
#define EXTERN
#endif

EXTERN void init_mbuff_queue(struct mb_queue *q);
EXTERN void init_msg_queue(struct msg_queue *q);
EXTERN void mbuff_enqueue (struct mb_queue *queue, struct mbuff *mb);
EXTERN void mbuff_push (struct mb_queue *queue, struct mbuff *mb);
EXTERN struct mbuff *mbuff_dequeue(struct mb_queue *queue);
EXTERN void mbuff_insert (struct mb_queue *queue, struct mbuff *mb);
EXTERN void mbuffmove (struct mb_queue *dst, struct mb_queue *src);
EXTERN void msg_enqueue (struct msg_queue *queue, struct msg *msg);
EXTERN struct msg *msg_dequeue(struct msg_queue *queue);
EXTERN void msgmove (struct msg_queue *dst, struct msg_queue *src);
EXTERN struct msg *msg_search(struct msg_queue *queue, struct mbuff *mb);
EXTERN struct msg *msg_search_custom(struct msg_queue *queue, void *data, int(*check)(void *, struct mbuff *) );
EXTERN size_t msgnmove (struct msg_queue *dst, struct msg_queue *src, size_t len);


#endif