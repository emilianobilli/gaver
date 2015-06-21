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
#ifndef _ITC_H
#define _ITC_H

#include <pthread.h>
#include <signal.h>
#include "mbuff.h"

/* 
 * La prioridad solamente afecta cuando los mensajes son de KERNEL_LAYER -> NETOUT_LAYER
 */
#define PRIO_CTR_QUEUE	 2	/* Prioridad para envio de mensajes de control */
#define PRIO_RET_QUEUE   1	/* Prioridad para la retransmision             */
#define PRIO_NOR_QUEUE	 0	/* Prioridad normal de mensajes		       */

/*
 * Inter Thread Comunication
 */
struct itc_msg_queue {
    struct msg_queue queue;
    pthread_mutex_t  mutex;
};

/*
 * Struct to read/write messages using ITC
 */
struct wrmsg {
    int src;
    int dst; 
    struct msg_queue *srcq;
    struct msg_queue *dstq;
    pthread_mutex_t  *msg_mutex;
    int signal;
};

#define THREAD_TABLE_SZ 4
#define APPDIO_LAYER_THREAD	0	/* Application Data Input/Output */
#define KERNEL_LAYER_THREAD	1	/* Core of Gaver */
#define NETOUT_LAYER_THREAD	2	/* Net Output (UDP Layer) */
#define NETINP_LAYER_THREAD	3	/* Net Input  (UDP Layer) */


/* Signal Events */
#define SE_KERTOAPP   SIGRTMIN		/* Signal Event: Message from KERNEL -> APP */
#define SE_KERTONET_0 SIGRTMIN+1	/* Signal Event: Message from KERNEL -> NET (Prio 0) */
#define SE_KERTONET_1 SIGRTMIN+2	/* Signal Event: Message from KERNEL -> NET (Prio 1) */
#define SE_KERTONET_2 SIGRTMIN+3	/* Signal Event: Message from KERNEL -> NET (Prio 2) */
#define SE_APPTOKER   SIGRTMIN+4	/* Signal Event: Message from APP -> KERNEL */
#define SE_NETTOKER   SIGRTMIN+5	/* Signal Event: Message from NET -> KERNEL */

/* Operations */
#define WR_OPT_READ	0
#define WR_OPT_WRITE	1

#endif /* itc.h */
