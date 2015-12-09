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
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include "timers.h"
#include "output.h"
#include "input.h"
#include "timers.h"
#include "kernel_util.h"
#include "kernel_defs.h"
#include "heap.h"
#include "sockopt.h"
#include "util.h"
#include "mbuff.h"
#include "mbuff_queue.h"
#include "kernel_api.h"
#include "types.h"
#include "glo.h"
#include "itc_var.h"
#include "defs.h"
#include "itc.h"
#ifdef DEBUG
#include "dump.h"
#endif


/*
 * Defines
 */
#define MAX_SOCKETS		256
#define MAX_EVENTS		MAX_SOCKETS + 1 + 1 + 1	/* Timer, Api Socket, Itc */
#define SOCKET_EVENTS		EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP
#define ITC_EVENTS		EPOLLIN
#define TIMER_EVENTS		EPOLLIN
#define SERVER_EVENTS		EPOLLIN | EPOLLERR


#define KEVENT_SOCKET_CLOSE(e) 	( ( (e) & EPOLLRDHUP ) || ( (e) & EPOLLHUP ) )
#define KEVENT_SOCKET_ERROR(e)	( (e) & EPOLLERR ) 
#define KEVENT_SERVER_ERROR(e)	( (e) & EPOLLERR )
#define KEVENT_ITC(e)		( (e) & EPOLLIN  )
#define KEVENT_TIMER(e)		( (e) & EPOLLIN  )
#define KEVENT_SOCKET(e)	( (e) & EPOLLIN  )
#define KEVENT_SERVER(e)	( (e) & EPOLLIN  )


/*
 * Global Private Vars
 */
PRIVATE  int keventfd;
PRIVATE  struct epoll_event kevents[MAX_EVENTS];
PRIVATE  struct epoll_event *pkev;


#define ADD 0
#define DEL 1

/*
 * FUNCTION PROTOTYPES
 */

/*======================================================================================*
 * kevent_init()									*
 *======================================================================================*/
PRIVATE void kevent_init(void);

/*======================================================================================*
 * kevent()										*
 *======================================================================================*/
PRIVATE int kevent(void);

/*======================================================================================*
 * kevent_wait()									*
 *======================================================================================*/
PRIVATE int kevent_wait(int timeout);

/*======================================================================================*
 * kevent_add_timer()									*
 *======================================================================================*/
PRIVATE int kevent_add_timer(int timersd);

/*======================================================================================*
 * kevent_add_itc()									*
 *======================================================================================*/
PRIVATE int kevent_add_itc(int itcsd);

/*======================================================================================*
 * kevent_add_socket_server()								*
 *======================================================================================*/
PRIVATE int kevent_add_socket_server(int sd);

/*======================================================================================*
 * kevent_add_socket()									*
 *======================================================================================*/
PRIVATE int kevent_add_socket(int socket);

/*======================================================================================*
 * kevent_del_socket()									*
 *======================================================================================*/
PRIVATE int kevent_del_socket(int socket);

/*======================================================================================*
 * kevent_add_del_socket()								*
 *======================================================================================*/
PRIVATE int kevent_add_del_socket(int socket, int op);

void *kernel(void *arg)
{
    struct sock *sk;		/* Sock Struct		*/
    u_int64_t ntimes;
    int nkevents;		/* Events Ready		*/
    int do_close;		/* If a socket fail	*/
    int i;
    ssize_t ret;
    char *where;		/* Panic		*/
    struct itc_event_info ieinfo;
    struct msg_queue tx_ctr_queue;	/* Output Control    */
    struct msg_queue tx_ret_queue;	/* Output Retransmit */
    struct msg_queue tx_nor_queue;	/* Output Normal     */
    struct msg_queue tx_input_queue;	/* Output Input	     */
    struct msg_queue rx_queue;		/* Input  Queue      */
    struct msg_queue io_queue_in;	/* Data IO Queue for Input  */
    struct msg_queue io_queue_out;	/* Data IO Queue for Output */

    arg = NULL;

    /* Add all Kernel Events  */
    kevent_init();
    kevent_add_timer(refresh_timer);
    kevent_add_itc(itc_event);
    kevent_add_socket_server(api_socket);

    
    /* Init all queues */
    init_msg_queue(&tx_ctr_queue);	/* Transmit */
    init_msg_queue(&tx_ret_queue);	/* Transmit */
    init_msg_queue(&tx_nor_queue);	/* Transmit */
    init_msg_queue(&tx_input_queue);	/* Retorno de transmit Internal */
    init_msg_queue(&rx_queue);		/* Reception */
    init_msg_queue(&io_queue_in);	/* Internal IO Read  */
    init_msg_queue(&io_queue_out);	/* Internal IO Write */

    /* Init sock Table */
    init_sock_table();

    /* Init expiration Timers */
    init_et();

#ifdef DEBUG
    dump_heap(stderr);
#endif

    thread_table[KERNEL_LAYER_THREAD] = pthread_self();
    /*
     * Start all Threads
     */
    pthread_create(&thread_table[NETOUT_LAYER_THREAD], NULL, output, NULL);
    pthread_create(&thread_table[NETINP_LAYER_THREAD], NULL, input, NULL);

    while(1) /* Kernel main loop */
    {
	/*
         * Wait for events
         */
	nkevents = kevent_wait(-1);

	for ( i = 0, pkev = &kevents[0]; i <= nkevents -1; i++, pkev++ )
	{
	    /* Look if a new socket is waiting for accept */
	    if ( pkev->data.fd == api_socket )
	    {
		if (KEVENT_SERVER(pkev->events))
		{
		    errno = 0;
		    sk = new_sk(api_socket);
		    if (sk != NULL)
			kevent_add_socket(sk->so_loctrl);
		    else
		    if ( errno == ENFILE  ||
			 errno == ENOBUFS ||
		         errno == ENOMEM  ||
			 errno == EPROTO ) {
		         where = "new_sk";
			 goto panic;
		    }
		}
		if (KEVENT_SERVER_ERROR(pkev->events))
		{	
		    where = "Api Server Error";
		    goto panic;
		}
	    }
	    /* Looks if is time to refresh tokens and syn */
	    else if ( pkev->data.fd == refresh_timer && 
		      KEVENT_TIMER(pkev->events) )
	    {
		errno = 0;
		/* Time to update tokens */
		if (gettimerexp(refresh_timer, &ntimes) == -1)
		{
		    where = "gettimerexp";
		    goto panic;
		}
		do_update_tokens(&so_used, ntimes);
	    }
	    /* Look if another thread send a message */

	    else if ( pkev->data.fd == itc_event && 
		      KEVENT_ITC(pkev->events) )
	    {
		ret = itc_read_event(itc_event, &ieinfo);
		if (ret == -1)
	        {
		    where = "itc_read_event()";
		    goto panic;
		}

		if (ieinfo.src == NETOUT_LAYER_THREAD) 
		    /*
		     * Net Out return a Package
		     */
		    itc_readfrom(NETOUT_LAYER_THREAD, &tx_input_queue, 0);
		

		if (ieinfo.src == NETINP_LAYER_THREAD)
		    /*
		     * Net Input Send a Package
		     */
		    itc_readfrom(NETINP_LAYER_THREAD, &rx_queue, 0);


		if (ieinfo.src == DATAIO_LAYER_THREAD)
		    /*
		     * Data IO send a Package
		     */
		    itc_readfrom(DATAIO_LAYER_THREAD, &io_queue_in, 0);
		/*
		 * Itc Event
                 */
	    }
	    else 
	    {
		do_close = 0;
		/* Look if soctrl send a api message */
		sk = getsockbysoctrl(pkev->data.fd);
		if (sk != NULL)
		{
		    if (KEVENT_SOCKET(pkev->events))
		    {
			/* Mensaje de api "kernel_api.c" */
			if(do_socket_request(sk,&tx_ctr_queue) == -1) {
			    do_close = 1;
			}
			else {
			    /* Hacer Algo */
			}
		    }
		    if (KEVENT_SOCKET_CLOSE(pkev->events) || do_close)
		    {
			/* Evaluar que paso */
			kevent_del_socket(pkev->data.fd);
		    }
		    if (KEVENT_SOCKET_ERROR(pkev->events))
		    {
			/* ?? */
		    }
		}
	    }
	} /* Check Events */

	/*
         * Finally: 
	 *	- Process All Input Queues
	 *	- Check if a socket have tokens available to send
         *	- Check if a socket needs to send syn message
	 *	- Check if a timer expire
	 *	- flush all queues
	 */
	do_process_input_bulk(&rx_queue, &tx_ctr_queue);
	do_process_sent_msg(&tx_input_queue);
	do_process_expired(&tx_ctr_queue);
	do_collect_mbuff_to_transmit(&so_used, &tx_nor_queue, &tx_ctr_queue);

	/* Flush all Output Queues */
	if (tx_ctr_queue.size)
	    itc_writeto(NETOUT_LAYER_THREAD, &tx_ctr_queue, PRIO_CTR_QUEUE);
	if (tx_ret_queue.size)
	    itc_writeto(NETOUT_LAYER_THREAD, &tx_ret_queue, PRIO_RET_QUEUE);
	if (tx_nor_queue.size)
	    itc_writeto(NETOUT_LAYER_THREAD, &tx_nor_queue, PRIO_NOR_QUEUE);
	
    }
panic:
    PANIC(errno,"kernel",where);
    return NULL;
}


/*======================================================================================*
 * kevent_init()									*
 *======================================================================================*/
void kevent_init(void)
{
    keventfd = kevent();
}


/*======================================================================================*
 * kevent()										*
 *======================================================================================*/
int kevent(void)
{
    return epoll_create(1);
}


/*======================================================================================*
 * kevent_wait()									*
 *======================================================================================*/
int kevent_wait(int timeout)
{
    return epoll_wait(keventfd, kevents, MAX_EVENTS ,timeout);
}




/*======================================================================================*
 * kevent_add_timer()									*
 *======================================================================================*/
int kevent_add_timer(int timersd)
{
    struct epoll_event ev;
    ev.events  = TIMER_EVENTS;
    ev.data.fd = timersd;

    return epoll_ctl(keventfd, EPOLL_CTL_ADD, timersd, &ev);
}

/*======================================================================================*
 * kevent_add_itc()									*
 *======================================================================================*/
int kevent_add_itc(int itcsd)
{
    struct epoll_event ev;
    ev.events  = ITC_EVENTS;
    ev.data.fd = itcsd;

    return epoll_ctl(keventfd, EPOLL_CTL_ADD, itcsd, &ev);
}

/*======================================================================================*
 * kevent_add_socket_server()								*
 *======================================================================================*/
int kevent_add_socket_server(int sd)
{
    struct epoll_event ev;
    ev.events  = SERVER_EVENTS;
    ev.data.fd = sd;

    return epoll_ctl(keventfd, EPOLL_CTL_ADD, sd, &ev);
}

/*======================================================================================*
 * kevent_add_socket()									*
 *======================================================================================*/
int kevent_add_socket(int socket)
{
    return kevent_add_del_socket(socket, ADD);
}

/*======================================================================================*
 * kevent_del_socket()									*
 *======================================================================================*/
int kevent_del_socket(int socket)
{
    return kevent_add_del_socket(socket, DEL);
}

/*======================================================================================*
 * kevent_add_del_socket()								*
 *======================================================================================*/
int kevent_add_del_socket(int socket, int op)
{
    struct epoll_event ev;
    ev.events  = SOCKET_EVENTS;
    ev.data.fd = socket;
    if (op == ADD)
    {
	return epoll_ctl(keventfd, EPOLL_CTL_ADD, socket, &ev);
    }
    if (op == DEL)
    {
	return epoll_ctl(keventfd, EPOLL_CTL_DEL, socket, &ev);
    }
    return -1;
}
