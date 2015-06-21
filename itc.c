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
#include "types.h"
#include "itc.h"
#include "itc_var.h"
#include "mbuff.h"
#include "mbuff_queue.h"

int getmsgprio(int sig)
{
    if (sig == SE_KERTOAPP || 
	sig == SE_APPTOKER ||
	sig == SE_NETTOKER ||
	sig == SE_KERTONET_0 )
	return 0;
    else {
	if (sig == SE_KERTONET_1)
	    return 1;
	else
	    if (sig == SE_KERTONET_2)
		return 2
    }
}



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

    return -1;
}


int writeto (int dst, struct msg_queue *q, int prio)
{
    return itc_wr_msgqueue(dst, q, prio, WR_OPT_WRITE);
}

int readfrom (int src, struct msg_queue *q, int prio)
{
    return itc_wr_msgqueue(src, q, prio, WR_OPT_READ);
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
int itc_wr_msgqueue ( int other, struct msg_queue *q, int prio, int opt )
{
    struct wrmsg wropt;

    switch (self())
    {
	case APPDIO_LAYER_THREAD:
	    if (other == KERNEL_LAYER_THREAD && opt == WR_OPT_WRITE)
	    {
		wropt.src  = APPDIO_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = q;
		wropt.dstq = &(appdio_kernel_queue.queue);
		wropt.msg_mutex = &(appdio_kernel_queue.mutex);
		wropt.signal = SE_APPTOKER;
	
		return writemsg(&wropt);
	    }
	    if (other == KERNEL_LAYER_THREAD && opt == WR_OPT_READ)
	    {
		wropt.src  = KERNEL_LAYER_THREAD;
		wropt.dst  = APPDIO_LAYER_THREAD;
		wropt.srcq = &(kernel_appdio_queue.queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(kernel_appdio_queue.mutex);
		wropt.signal = -1;

		return readmsg(&wropt);
	    }
	    break;
	case KERNEL_LAYER_THREAD:
	    if (other == APPDIO_LAYER_THREAD && opt == WR_OPT_WRITE)
	    {
		wropt.src  = KERNEL_LAYER_THREAD;
		wropt.dst  = APPDIO_LAYER_THREAD;
		wropt.srcq = q;
		wropt.dstq = &(kernel_appdio_queue.queue);
		wropt.msg_mutex = &(kernel_appdio_queue.mutex);
		wropt.signal = SE_KERTOAPP;
    
		return writemsg(&wropt);
	    }

	    if (other == APPDIO_LAYER_THREAD && opt == WR_OPT_READ)
	    {
		wropt.src  = APPDIO_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = &(appdio_kernel_queue.queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(appdio_kernel_queue.mutex);
		wropt.signal = -1;

		return reademsg(&wropt);
	    }

	    if (other == NETOUT_LAYER_THREAD && opt == WR_OPT_WRITE)
	    {
		wropt.src  = KERNEL_LAYER_THREAD;
		wropt.dst  = NETOUT_LAYER_THREAD;
		wropt.srcq = q;
		wropt.dstq = &(kernel_netout_queue[prio].queue);
		wropt.msg_mutex = &(kernel_netout_queue[prio].mutex);
		wropt.signal = (prio == 0) ? SE_KERTONET_0 : ((prio == 1) ? SE_KERTONET_1 : SE_KERTONET_2);
    
		return writemsg(&wropt);
	    }
	    if (other == NETINP_LAYER_THREAD && opt == WR_OPT_READ)
	    {
		wropt.src  = NETINP_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = &(netinp_kernel_queue.queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(netinp_kernel_queue.mutex);
		wropt.signal = -1;
	    
		return readmsg(&wropt);
	    }

	    break;
	case NETINP_LAYER_THREAD:
	    if (other == KERNEL_LAYER_THREAD && WR_OPT_WRITE)
	    {
		wropt.src  = NETIMP_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = q;
		wropt.dstq = &(netinp_kernel_queue.queue);
		wropt.msg_mutex = &(netinp_kernel_queue.mutex);
		wropt.signal = SE_NETTOKER

		return writemsg(&wropt);
	    }
	    break;
	case NETOUT_LAYER_THREAD:
	    if (other == KERNEL_LAYER_THREAD && WR_OPT_READ)
	    {
		wropt.src  = NETINP_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = &(kernel_netout_queue[prio].queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(kernel_netout_queue[prio].mutex);
		wropt.signal = -1;
	    
		return readmsg(&wropt);
	    }
	    break;
    }

}



int writemsg (struct wrmsg *opt)
{
    union sigval sval;

    sval.sival_int = opt->src;

    pthread_mutex_lock(opt->msg_mutex);
    msgqcat(opt->dstq, opt->srcq);
    pthread_mutex_unlock(opt->msg_mutex);

    pthread_sigqueue(thread_table[opt->dst],opt->signal, sval);

    return 0;
}


int readmsg (struct wrmsg *opt)
{
    pthread_mutex_lock(opt->msg_mutex);
    msgqcat(opt->dstq, opt->srcq);
    opt->srcq->head = NULL;
    opt->srcq->tail = NULL;
    pthread_mutex_unlock(opt->msg_mutex);

    return 0;
}

