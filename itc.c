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
#include <unistd.h>
#include "types.h"
#include "itc.h"
#include "itc_var.h"
#include "mbuff.h"
#include "mbuff_queue.h"
#include <signal.h>


/*----------------------------------------------------------------------------------------------*
 * itc_msg_queue_init(): Inicializa una cola de mensajes					*
 * 												*
 * Nota: Se deben inicializar todas las colas una vez para todos los hilos			*
 *----------------------------------------------------------------------------------------------*/
void itc_msg_queue_init(struct itc_msg_queue *q)
{
    q->queue.head = NULL;
    q->queue.tail = NULL;
    pthread_mutex_init(&(q->mutex), NULL);
}

/*----------------------------------------------------------------------------------------------*
 * itc_signalfd_init(): Return a file descriptor for the itc signals				*
 * 												*
 * Nota: Se debe realizar una sola inicializacion para todos los hilos				*
 *----------------------------------------------------------------------------------------------*/
int itc_signalfd_init (void)
{
    sigset_t seset;
    sigemptyset(&seset);    
    sigaddset(&seset,SE_KERTOAPP);
    sigaddset(&seset,SE_KERTONET_0);
    sigaddset(&seset,SE_KERTONET_1);
    sigaddset(&seset,SE_KERTONET_2);
    sigaddset(&seset,SE_APPTOKER);
    sigaddset(&seset,SE_NETTOKER);

    return signalfd(-1, &seset, 0);
}

/*----------------------------------------------------------------------------------------------*
 * itc_block_signal(): Block the signals for itc comunication					*
 *												*
 * Nota: Cada hilo debe bloquear las señales							*
 *----------------------------------------------------------------------------------------------*/
int itc_block_signal (void)
{
    sigset_t seset;
    sigemptyset(&seset);    
    sigaddset(&seset,SE_KERTOAPP);
    sigaddset(&seset,SE_KERTONET_0);
    sigaddset(&seset,SE_KERTONET_1);
    sigaddset(&seset,SE_KERTONET_2);
    sigaddset(&seset,SE_APPTOKER);
    sigaddset(&seset,SE_NETTOKER);

    return pthread_sigmask(SIG_BLOCK, &seset, NULL);
}

/*----------------------------------------------------------------------------------------------*
 * itc_read_event(fd, info): Lee el evento (msg) que recibio el hilo.				*
 *			     La estructura info trae la informacion del hilo emisor y la prio	*
 *----------------------------------------------------------------------------------------------*/
int itc_read_event(int fd, itc_event_info *info)
{
    struct signalfd_siginfo siginfo;
    ssize_t len;    

    len = read(fd, &siginfo, sizeof(struct signalfd_siginfo));

    if ( len != -1 ){
	info->src  = siginfo.ssi_int;
	info->prio = itc_getmsgprio(siginfo.ssi_signo);
    }
    return len;
}

/*----------------------------------------------------------------------------------------------*
 * itc_getmsgprio(sig): Captura la prioridad del mensaje dependiendo la signal usada por el	*
 *                      emisor									*
 *----------------------------------------------------------------------------------------------*/
int itc_getmsgprio(int sig)
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

/*----------------------------------------------------------------------------------------------*
 * itc_self(): Retorna el indice con el Rol del thread que hace la invoca			*
 *----------------------------------------------------------------------------------------------*/
int itc_self(void)
{
    int i;
    pthread_t self = pthread_self();
    
    for ( i = 0; i <= THREAD_TABLE_SZ-1; i++)
	if (pthread_equal(self, thread_table[i]))
	    return i;

    return -1;
}


int itc_writeto (int dst, struct msg_queue *q, int prio)
{
    return itc_wr_msgqueue(dst, q, prio, WR_OPT_WRITE);
}

int itc_readfrom (int src, struct msg_queue *q, int prio)
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

    switch (itc_self())
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
	
		return itc_writemsg(&wropt);
	    }
	    if (other == KERNEL_LAYER_THREAD && opt == WR_OPT_READ)
	    {
		wropt.src  = KERNEL_LAYER_THREAD;
		wropt.dst  = APPDIO_LAYER_THREAD;
		wropt.srcq = &(kernel_appdio_queue.queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(kernel_appdio_queue.mutex);
		wropt.signal = -1;	/* Valor no usado */

		return itc_readmsg(&wropt);
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
    
		return itc_writemsg(&wropt);
	    }

	    if (other == APPDIO_LAYER_THREAD && opt == WR_OPT_READ)
	    {
		wropt.src  = APPDIO_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = &(appdio_kernel_queue.queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(appdio_kernel_queue.mutex);
		wropt.signal = -1;	/* Valor no usado para lectura */

		return itc_reademsg(&wropt);
	    }

	    if (other == NETOUT_LAYER_THREAD && opt == WR_OPT_WRITE)
	    {
		wropt.src  = KERNEL_LAYER_THREAD;
		wropt.dst  = NETOUT_LAYER_THREAD;
		wropt.srcq = q;
		wropt.dstq = &(kernel_netout_queue[prio].queue);
		wropt.msg_mutex = &(kernel_netout_queue[prio].mutex);
		wropt.signal = (prio == 0) ? SE_KERTONET_0 : ((prio == 1) ? SE_KERTONET_1 : SE_KERTONET_2);
    
		return itc_writemsg(&wropt);
	    }
	    if (other == NETINP_LAYER_THREAD && opt == WR_OPT_READ)
	    {
		wropt.src  = NETINP_LAYER_THREAD;
		wropt.dst  = KERNEL_LAYER_THREAD;
		wropt.srcq = &(netinp_kernel_queue.queue);
		wropt.dstq = q;
		wropt.msg_mutex = &(netinp_kernel_queue.mutex);
		wropt.signal = -1;	/* Valor no usado para lectura */
	    
		return itc_readmsg(&wropt);
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

		return itc_writemsg(&wropt);
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
		wropt.signal = -1;	/* Valor no usado para lectura */
	    
		return itc_readmsg(&wropt);
	    }
	    break;
    }

}

int itc_writemsg (struct wrmsg *opt)
{
    union sigval sval;
    int send_signal = 0;

    sval.sival_int = opt->src;

    pthread_mutex_lock(opt->msg_mutex);
    if (opt->dstq->head == NULL && opt->dstq->tail == NULL)
	send_signal = 1;
    msgqcat(opt->dstq, opt->srcq);
    pthread_mutex_unlock(opt->msg_mutex);
    
    if (send_signal)
	/*
	 * Solamente se envia la signal si el hilo receptor tiene la cola vacia
	 * tener datos en la cola implica que ya se envio una signal y el hilo
	 * todavia no consumio los datos
	 */
	pthread_sigqueue(thread_table[opt->dst],opt->signal, sval);

    return 0;
}


int itc_readmsg (struct wrmsg *opt)
{
    pthread_mutex_lock(opt->msg_mutex);
    msgqcat(opt->dstq, opt->srcq);
    opt->srcq->head = NULL;
    opt->srcq->tail = NULL;
    pthread_mutex_unlock(opt->msg_mutex);

    return 0;
}

