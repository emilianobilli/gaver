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
#ifndef _HEAP_H
#define _HEAP_H
#include "heap_var.h"
#include "mbuff.h"

#ifdef _HEAP_CODE
#undef EXTERN
#define EXTERN
#endif

EXTERN void free_mbuff(struct mbuff *m);
EXTERN void free_mbuff_locking(struct mbuff *m);

EXTERN void free_msg(struct msg *p);
EXTERN void free_msg_locking(struct msg *p);

EXTERN struct mbuff *alloc_mbuff(void);
EXTERN struct mbuff *alloc_mbuff_locking(void);
EXTERN size_t alloc_mbuff_chain( struct msg_queue *queue, size_t len);
EXTERN struct mbuff *clone_mbuff(struct mbuff *mb);
EXTERN size_t alloc_msg_chain( struct msg_queue *queue, size_t len);

EXTERN struct msg *alloc_msg(void);
EXTERN struct msg *alloc_msg_locking(void);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_mbuff_queue()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN size_t alloc_mbuff_queue (struct mb_queue *queue, size_t len);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * alloc_msg_queue()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN size_t alloc_msg_queue (struct msg_queue *queue, size_t len);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * heap_mbuff_size()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN size_t heap_mbuff_size(void);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * heap_msg_size()									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN size_t heap_msg_size(void);


EXTERN size_t alloc_mbuff_payload( struct mb_queue *queue, size_t len, size_t payload_size);
EXTERN void init_heap(size_t len);

#endif

