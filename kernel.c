#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "sockopt.h"
#include "types.h"
#include "glo.h"
#include "itc_var.h"
#include "itc.h"


#define MAX_SOCKETS		256
#define MAX_EVENTS		MAX_SOCKETS + 1 + 1 + 1	/* Timer, Api Socket, Itc */
#define SOCKET_EVENTS		EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP
#define ITC_EVENTS		EPOLLIN
#define TIMER_EVENTS		EPOLLIN
#define SERVER_EVENTS		EPOLLIN | EPOLLERR


#define KEVENT_SOCKET_ERROR(e)		( ( (e) & EPOLLRDHUP ) || ( (e) & EPOLLERR ) || ( (e) & EPOLLHUP ) )
#define KEVENT_SERVER_ERROR(e)		( (e) & EPOLLERR )
#define KEVENT_ITC(e)			( (e) & EPOLLIN  )
#define KEVENT_TIMER(e)			( (e) & EPOLLIN  )
#define KEVENT_SOCKET(e)		( (e) & EPOLLIN  )
#define KEVENT_SERVER(e)		( (e) & EPOLLIN  )



    while(1)
    {
	/*
         * Wait for events
         */
	nkevens = kevent_wait(-1);

	for ( i = 0, pkev = &kevent[0]; i <= nkevents -1; i++, pkev++ )
	{
	    /*
             * Look if a new socket is waiting for accept
             */
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
		else if (KEVENT_SERVER_ERROR(pkev->events))
		{	
		/*
        	 * ToDo Manejar los Panic
    	         */
		    panic();
		}
	    }
	    else if ( pkev->data.fd == refresh_timer && 
		      KEVENT_TIMER(pkev->events) )
	    {
		/*
		 * Time to update tokens
		 */
		if (gettimerexp(refresh_timer, &ntimes) == -1)
		{
		    panic();
		}
		do_update_tokens(&so_used, ntimes);

	    }
	    else if ( pkev->data.fd == itc_event && 
		      KEVENT_ITC(pkev->events) )
	    {
		/*
		 * Itc Event
                 */


	    }
	}
/*
 * This is the JSON message when the connection is established with the api
 */
#define JSON_TEMPLATE "{\"Gaver\":\"%s\",\"Status\":\"%d\",\"Reason\":\"%s\"}";
#define JSON_BUFFER   512


EXTERN  int keventfd;
EXTERN  struct epoll_event kevents[MAX_EVENTS];
EXTERN  struct epoll_event *pkev;

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
	    nsk->so_loctrl_state = CTRL_CLOSE;
	    nsk->so_state        = GV_CLOSE;
	    nsk->so_lodata	 = -1;
	    nsk->so_lodata_statu = DATA_IO_NONE;
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

void do_update_tokens (struct sockqueue *sk, u_int64_t times)
{
    struct sockqueue tmp;
    struct sock *ptr;

    init_sock_queue(&tmp);
    
    /*
	Part of the sock structure
	--------------------------
	double  	so_refresh_syn;
        double		so_refresh_tokens;
        double		so_available_tokens;
        u_int8_t	so_capwin;	
	size_t		so_hostwin;		

	struct msg_queue 

    */
    while (ptr = sock_dequeue(sk))
    {
	if (ptr->so_state == GV_ESTABLISHED)
	{
	    if (ptr->so_available_tokens > 1.0)
		/*
    		 * Se le resta la parte entera
                 */
		ptr->so_available_tokens = ptr->so_available_tokens - floor(ptr->so_available_tokens);

	    /* 
    	    * At this point the value of so_available_tokens is in the set [0;1)
            */
	
	    ptr->so_available_tokens += update_token(ptr->so_available_tokens,
						     ptr->so_refresh_toekns,
						     ptr->so_capwin,
						     ptr->so_hostwin-ptr->so_sentq.size) * (double) times;

	    ptr->so_refresh_syn += (ptr->so_refresh_tokens / PACKET_PER_ROUND) * (double) times;
	    /*
             * When so_refresh_syn >= 1 -> The kernel dispatch the syn msg and substract 
	     *			       the integer value
	     *
             */
	}
	sock_enqueue(&tmp,ptr);
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

#define ADD 0
#define DEL 1


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

