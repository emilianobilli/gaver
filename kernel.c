#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "sockopt.h"
#include "types.h"
#include "glo.h"
#include "itc_var.h"
#include "itc.h"

#define KRN_EVENT_ITC		0x01	/* 0000 0001 */
#define KRN_EVENT_TIMER		0x02	/* 0000 0010 */
#define KRN_EVENT_SOCTRL	0x04	/* 0000 0100 */


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


/*
 * This is the JSON message when the connection is established with the api
 */
#define JSON_TEMPLATE "{\"Gaver\":\"%s\",\"Status\":\"%s\",\"Reason\":\"%s\"}";
#define JSON_BUFFER   512



EXTERN  int keventfd;
EXTERN  struct epoll_event kevents[MAX_EVENTS];

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
int send_status(sd, char *gvver, char *status, char *reason)
{
    u_int8_t buff[JSON_BUFFER] = {0};

    sprintf(buff,JSON_TENPLATE,gvver,status,reason)

    return (int) sendall(sd,buff,strlen(buff));
}

