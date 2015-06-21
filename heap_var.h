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

EXTERN struct msg_queue heap_msg;
EXTERN pthread_mutex_t  heap_msg_mutex;

#endif /* _HEAP_VAR_H */

#ifndef _ITC_VAR_H
#define _ITC_VAR_H

#include "itc.h"

EXTERN struct itc_msg_queue appdio_kernel_queue;	/* Messages FROM App TO Kernel */
EXTERN struct itc_msg_queue kernel_appdio_queue;	/* Messages FROM Kernel TO App */	
EXTERN struct itc_msg_queue kernel_netout_queue[3];	/* Messages FROM Kernel TO Network Output */	
EXTERN struct itc_msg_queue netinp_kernel_queue;	/* Messages FROM Network Input to Kernel */

EXTERN pthread_t thread_table[THREAD_TABLE_SZ];

#endif

#ifndef _ITC_H
#define _ITC_H

#define CONTROL_QUEUE	 0
#define RETRANSMIT_QUEUE 1
#define NORMAL_QUEUE	 2
/*
 * Inter Thread Comunication
 */
struct itc_msg_queue {
    struct msg_queue queue;
    pthread_mutex_t  mutex;
};

#define THREAD_TABLE_SZ 4
#define APPDIO_LAYER_THREAD	0	/* Application Data Input/Output */
#define KERNEL_LAYER_THREAD	1	/* Core of Gaver */
#define NETOUT_LAYER_THREAD	2	/* Net Output (UDP Layer) */
#define NETINP_LAYER_THREAD	3	/* Net Input  (UDP Layer) */

#define SIGITC SIGRTMIN


#endif

void itc_msg_queue_init(struct itc_msg_queue *q)
{
    q->queue.head = NULL;
    q->queue.tail = NULL;
    pthread_mutex_init(&(q->mutex), NULL);
}


/*------------------------------------------------------------------------------------------------------*
 * self(): 												*
 *	Retorna el indice con el Rol del thread que hace la invoca					*
 *------------------------------------------------------------------------------------------------------*/
int self(void)
{
    int i;
    pthread_t self = pthread_self();
    
    for ( i = 0; i <= THREAD_TABLE_SZ-1; i++)
	if (pthread_equal(self, thread_table[i]))
	    return i;
}

/*------------------------------------------------------------------------------------------------------*
 * writeto(int dst, struct msg_queue *q, int prio):							*
 * 	Concatena una lista de nodos msg en una cola especifica para el receptor			*
 *													*
 *	Parametros:											*
 *		- dst: Especifica a que ROL de THREAD se deben concantenar lo mensajes			*
 *		- q: La cola que se debe concatenar							*
 *		- prio: En que prioridad se debe concatenar, se ignora si el destinatario no 		*
 *			tiene colas por prioridad							*
 *------------------------------------------------------------------------------------------------------*/
void writeto (int dst, struct msg_queue *q, int prio)
{
    int src = self();
    union sigval sval;

    switch(src)
    {
	case APPDIO_LAYER_THREAD:
	    /*
             * APPDIO_LAYER_THREAD -> KERNEL_LAYER_THREAD
             */
    	    sval.sival_int = APPDIO_LAYER_THREAD;
	    if (dst == KERNEL_LAYER_THREAD) 
	    {
		pthread_mutex_lock(&(appdio_kernel_queue.mutex));
		msgqcat(&(appdio_kernel_queue.queue), q);
		pthread_mutex_unlock(&(appdio_krenel_queue.mutex));
		pthread_sigqueue(thread_table[KERNEL_LAYER_THREAD], SIGITC, sval);
	    }
	    break;

	case KERNEL_LAYER_THREAD:
	    /*
             * KERNEL_LAYER_THREAD -> APPDIO_LAYER_THREAD
             */

	    sval.sival_int = KERNEL_LAYER_THREAD;
	    if (dst == APPDIO_LAYER_THREAD)
	    {
		pthread_mutex_lock(&(kernel_appdio_queue.mutex));
		msgqcat(&(kernel_appdio_queue.queue), q);
		pthread_mutex_unlock(&(kernel_appdio_queue.mutex));
		pthread_sigqueue(thread_table[APPDIO_LAYER_THREAD], SIGITC, sval);
	    }
	    
	    /*
	     * KERNEL_LAYER_THREAD -> NETOUT_LAYER_THREAD
	     */

	    if (dst == NETOUT_LAYER_THREAD)
	    {
		pthread_mutex_lock(&(kernel_netout_queue[prio].mutex));
		msgqcat(&(kernel_netout_queue[prio].queue), q);
		pthread_mutex_unlock(&(kernel_netout_queue[prio].mutex));
		pthread_sigqueue(thread_table[NETOUT_LAYER_THREAD], SIGITC, sval);
	    }

	    break;
	case NETINP_LAYER_THREAD:

	    /*
	     * NETINP_LAYER_THREAD -> KERNEL_LAYER_THREAD
	     */

	    sval.sival_int = NETINP_LAYER_THREAD;
	    if (dst == KERNEL_LAYER_THREAD)
	    {
		pthread_mutex_lock(&(netinp_kernel_queue.mutex));
		msgqcat(&(netinp_kernel_queue.queue), q);
		pthread_mutex_unlock(&(netinp_kernel_queue.mutex));
		pthread_sigqueue(thread_table[KERNEL_LAYER_THREAD], SIGITC, sval);
	    }
	    break;
	default:
	    break;
    }
}


int readfrom(int src, struct msg_queue *q, int prio)
{


}