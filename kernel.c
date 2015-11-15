#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "sockopt.h"
#include "kernel_api.h"
#include "types.h"
#include "glo.h"
#include "itc_var.h"
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


#define KEVENT_SOCKET_CLOSE(e) 		( ( (e) & EPOLLRDHUP ) || ( (e) & EPOLLHUP ) )
#define KEVENT_SOCKET_ERROR(e)		( (e) & EPOLLERR ) 
#define KEVENT_SERVER_ERROR(e)		( (e) & EPOLLERR )
#define KEVENT_ITC(e)			( (e) & EPOLLIN  )
#define KEVENT_TIMER(e)			( (e) & EPOLLIN  )
#define KEVENT_SOCKET(e)		( (e) & EPOLLIN  )
#define KEVENT_SERVER(e)		( (e) & EPOLLIN  )

/*
 * This is the JSON message when the connection is established with the api
 */
#define JSON_TEMPLATE "{\"Gaver\":\"%s\",\"Status\":\"%d\",\"Reason\":\"%s\"}";
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
 * pfloat()										*
 *======================================================================================*/
PRIVATE double pfloat (double value);

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
PRIVATE int send_status(sd, int status, char *reason);




    while(1)
    {
	/*
         * Wait for events
         */
	nkevens = kevent_wait(-1);

	for ( i = 0, pkev = &kevent[0]; i <= nkevents -1; i++, pkev++ )
	{
	    /* Look if a new socket is waiting for accept */
	    if ( pkev->data.fd == server_api )
	    {
		if (KEVENT_SERVER(pkev->events))
		{
		    errno = 0;
		    sock = new_sk(server_api);
		    if (sock != NULL)
			kevent_add_socket(sock->so_loctrl);
		    else
		    if ( errno == ENFILE  ||
			 errno == ENOBUFS ||
		         errno == ENOMEM  ||
			 errno == EPROT )
		         panic();
		}
		if (KEVENT_SERVER_ERROR(pkev->events))
		{	
		/* ToDo Manejar los Panic */
		    panic();
		}
	    }
	    /* Looks if is time to refresh tokens and syn */
	    else if ( pkev->data.fd == refresh_timer && 
		      KEVENT_TIMER(pkev->events) )
	    {
		/* Time to update tokens */
		if (gettimerexp(refresh_timer, &ntimes) == -1)
		{
		    panic();
		}
		do_update_tokens(&so_used, ntimes);
	    }
	    /* Look if another thread send a message */
	    else if ( pkev->data.fd == itc_event && 
		      KEVENT_ITC(pkev->events) )
	    {
		/*
		 * Itc Event
                 */
	    }
	    else 
	    {
		/* Look if soctrl send a api message */
		sock = getsockbysoctrl(pkev->data.fd);
		if (sock != NULL)
		{
		    if (KEVENT_SOCKET(pkev->events))
		    {
			/* Mensaje de api */
		    }
		    if (KEVENT_SOCKET_CLOSE(pkev->events))
		    {
			/* Evaluar que paso */
		    }
		    if (KEVENT_SOCKET_ERROR(pkev->events))
		    {
			/* ?? */
		    }
		}
	    }
	/*
         * Finally: 
	 *	- Check if a socket have tokens available to send
         *	- Check if a socket needs to send syn message
	 *	- Check if a timer expire
	 *	- flush all queues
	 */

	}





/*======================================================================================*
 * new_sk()										*
 *======================================================================================*/
struct sock *new_sk( int sd )
{
    struct sock *nsk = NULL;		/* New Socket */
    int nsd;				/* New Socket Descriptor */

    nsd = accept(sd,(struct sockaddr *)NULL, (socklen_t *)NULL);
    if ( nsd != -1 )
    {
	nsk = getfreesock();
	if (nsk != NULL)
	{
	    init_sock(nsk);
	    setusedsock(nsk);
	    nsk->so_loctrl       = nsd;
	    nsk->so_loctrl_state = CTRL_NONE;
	    nsk->so_state        = GV_CLOSE;
	    nsk->so_lodata	 = -1;
	    nsk->so_lodata_state = DATA_IO_NONE;
	    nsk->so_local_gvport = NO_GVPORT;
	    send_status(nsd, 0, "Sucess");
	}
	else 
	{
	    send_status(nds,-1, "Maximun Open Gv Socket Reached");
	    close(nsd);
	}
    }
    return nsk;
}

/*======================================================================================*
 * pfloat()										*
 *======================================================================================*/
double pfloat (double value)
{
    return value - floor(value);
}

/*======================================================================================*
 * do_update_tokens_sock()									*
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
	    ptr->so_avtok = pfloat(ptr->so_avtok);
	    /* At this point the value of so_available_tokens is in the set [0;1) */
	hw = sk->so_hostwin;		/* Host Window */
	sq = sk->so_sentq.size;		/* Sent Queue  */
	if ( hw > sq )
	    sk->so_avtok += update_token(sk->so_avtok,
					 sk->so_retok,
					 sk->so_capwin,
					 hw-sq) * (double) times;
	else
	    /* The other alternative is that the value is 0 */
	    sk->so_avtok = 0;
	
	sk->so_resyn += (ptr->so_retok / PACKET_PER_ROUND) * (double) times;
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
        
    while (skp = sock_dequeue(sk)) {
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
    ev.data.fd = timersd;

    return epoll_ctl(keventfd, EPOLL_CTL_ADD, itcsd, &ev);
}

/*======================================================================================*
 * kevent_add_socket_server()								*
 *======================================================================================*/
int kevent_add_socket_server(int sd)
{
    struct epoll_event ev;
    ev.events  = SERVER_EVENTS;
    ev.data.fd = timersd;

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
	ret = send(sd, &ptr[sent], len - sent, MSG_NOSIGNAL)
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
int send_status(sd, int status, char *reason)
{
    u_int8_t buff[JSON_BUFFER] = {0};

    sprintf(buff,JSON_TENPLATE,GAVER_VERSION,status,reason);

    return (int) sendall(sd,buff,strlen(buff));
}

