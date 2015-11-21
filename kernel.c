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
#include <math.h>
#include <errno.h>
#include <unistd.h>
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
 * This is the JSON message when the connection is established with the api
 */
#define JSON_TEMPLATE "{\"Gaver\":\"%s\",\"Status\":\"%d\",\"Reason\":\"%s\"}"
#define JSON_BUFFER   512


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
 * new_sk()										*
 *======================================================================================*/
PRIVATE struct sock *new_sk( int sd );

/*======================================================================================*
 * do_update_tokens_sock()								*
 *======================================================================================*/
PRIVATE void do_update_tokens_sock (struct sock *sk, u_int64_t times);

/*======================================================================================*
 * kevent_init()									*
 *======================================================================================*/
PRIVATE void do_update_tokens (struct sockqueue *sk, u_int64_t times);

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

/*======================================================================================*
 * sendall()										*
 *======================================================================================*/
PRIVATE ssize_t sendall (int sd, void *buff, size_t len);

/*======================================================================================*
 * send_status()									*
 *======================================================================================*/
PRIVATE int send_status(int sd, int status, char *reason);


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
    init_msg_queue(&tx_input_queue);	/* Internal */
    init_msg_queue(&rx_queue);		/* Reception */
    init_msg_queue(&io_queue_in);	/* Internal IO Read  */
    init_msg_queue(&io_queue_out);	/* Internal IO Write */

    /* Init sock Table */
    init_sock_table();

    thread_table[KERNEL_LAYER_THREAD] = pthread_self();
    /*
     * Start all Threads
     */

    while(1)
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
			/* Mensaje de api */
			if(do_socket_request(sk,NULL) == -1)
			    do_close = 1;
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
	/*
         * Finally: 
	 *	- Process All Input Queues
	 *	- Check if a socket have tokens available to send
         *	- Check if a socket needs to send syn message
	 *	- Check if a timer expire
	 *	- flush all queues
	 */
	}
	/*
	 * Flush all Output Queues
	 */
    }
panic:
    PANIC(errno,"kernel",where);
    return NULL;
}




/*======================================================================================*
 * new_sk()										*
 *======================================================================================*/
struct sock *new_sk( int sd )
{
    struct sock *nsk = NULL;		/* New Socket */
    u_int64_t speed;
    int nsd;				/* New Socket Descriptor */

    nsd = accept(sd,(struct sockaddr *)NULL, (socklen_t *)NULL);
    if ( nsd != -1 )
    {
	if (!free_bps)
	{
	    send_status (nsd,-1,"Not speed available");
	    close(nsd);
	}
	else
	    speed = (free_bps > socket_bps) ? socket_bps : free_bps;

	nsk = getfreesock();
	if (nsk != NULL)
	{
	    init_sock(nsk);
	    setusedsock(nsk);
	    nsk->so_loctrl       = nsd;
	    nsk->so_loctrl_state = CTRL_NONE;
	    nsk->so_state        = GV_CLOSE;
	    nsk->so_lodata	 = -1;
	    nsk->so_capwin	 = START_CAPWIN;		/* Start Congestion Avoidance */

	    nsk->so_resyn	 = 0;
	    nsk->so_avtok	 = 0;
	    
    	    nsk->so_retok	 = getreftime(speed, mtu);	/* Cantidad de tokens a actualizar */

	    nsk->so_mtu		 = mtu;				/* Global MTU */
	    nsk->so_speed	 = speed;			/* Configured Speed */

	    free_bps 		-= speed;			/* Update available speed */

	    nsk->so_lodata_state = DATA_IO_NONE;
	    nsk->so_local_gvport = NO_GVPORT;
	    send_status(nsd, 0, "Sucess");
	}
	else 
	{
	    send_status(nsd,-1, "Maximun Open Gv Socket Reached");
	    close(nsd);
	}
    }
    return nsk;
}

/*======================================================================================*
 * do_update_tokens_sock()								*
 *======================================================================================*/
void do_update_tokens_sock (struct sock *sk, u_int64_t times)
{
    size_t hw, sq;
    /*
	Part of the sock structure
	--------------------------
	double  	so_resyn;
        double		so_retok;
        double		so_avtok;
        u_int8_t	so_capwin;	
	size_t		so_hostwin;		

	struct msg_queue so_sentq;
    */

    if (sk->so_state == GV_ESTABLISHED) 
    {
	if (sk->so_avtok > (double) 1.0)
	    /* Se le resta la parte entera */
	    sk->so_avtok = pfloat(sk->so_avtok);

	/* At this point the value of so_available_tokens is in the set [0;1) */
	hw = sk->so_host_win;		/* Host Window */
	sq = sk->so_sentq.size;		/* Sent Queue  */
	if ( hw > sq )
	    sk->so_avtok += update_token(sk->so_avtok,
					 sk->so_retok,
					 sk->so_capwin,
					 hw-sq) * (double) times;
	else
	    /* The other alternative is that the value is 0 */
	    sk->so_avtok = 0;
	
	sk->so_resyn += (sk->so_retok / PACKETS_PER_ROUND) * (double) times;
	/*
         * When so_refresh_syn >= 1 -> The kernel dispatch the syn msg and substract 
	 *			       the integer value
	 *
         */
    }
    return;
}

/*======================================================================================*
 * kevent_init()									*
 *======================================================================================*/
void do_update_tokens (struct sockqueue *sk, u_int64_t times)
{
    struct sockqueue tmp;
    struct sock *skp;
    
    init_sock_queue(&tmp);
        
    while ( (skp = sock_dequeue(sk)) ) {
	do_update_tokens_sock(skp,times);
	sock_enqueue(&tmp,skp);
    }
    sk->size = tmp.size;
    sk->head = tmp.head;
    sk->tail = tmp.tail;
    return;
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



/*======================================================================================*
 * sendall()										*
 *======================================================================================*/
ssize_t sendall (int sd, void *buff, size_t len)
{
    u_int8_t *ptr;
    ssize_t   ret;
    size_t    sent;
    
    sent = 0;

    ptr = buff;
    while (sent < len)
    {
	ret = send(sd, &ptr[sent], len - sent, MSG_NOSIGNAL);
	if (ret == -1)
	{
	    if (errno == EINTR)
		continue;
	}
	else
	    return (ssize_t) -1;
	sent +=ret;
    }
    return (ssize_t) sent;
}

/*======================================================================================*
 * send_status()									*
 *======================================================================================*/
int send_status(int sd, int status, char *reason)
{
    u_int8_t buff[JSON_BUFFER] = {0};

    sprintf((char *)buff,JSON_TEMPLATE,GAVER_VERSION,status,reason);

    return (int) sendall(sd,buff,strlen((char *)buff));
}

