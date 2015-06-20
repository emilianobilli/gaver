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

EXTERN void free_mbuff(struct mbuff *m);
EXTERN void free_mbuff_locking(struct mbuff *m);

EXTERN void free_pkt(struct pktbuff *p);
EXTERN void free_pkt_locking(struct pktbuff *p);

EXTERN struct mbuff *alloc_mbuff(void);
EXTERN struct mbuff *alloc_mbuff_locking(void);
EXTERN int alloc_mbuff_chain( struct pktqueue *queue, size_t len);

EXTERN struct pktbuff *alloc_pkt(void);
EXTERN struct pktbuff *alloc_pkt_locking(void);

EXTERN void init_heap(void);

#endif

