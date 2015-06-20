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

EXTERN struct pkt_queue heap_pktbuff;
EXTERN pthread_mutex_t  heap_pkt_mutex;

#endif /* _HEAP_VAR_H */
