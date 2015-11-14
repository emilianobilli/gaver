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

#define _SOCK_CODE
#include "sock.h"
#include "sock_var.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock_table(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_sock_table(void)
{
    int n;

    init_sock_queue(&so_used);
    init_sock_queue(&so_free);

    for ( n = 0; n <= MAX_SOCKETS-1; n++ )
    {
	init_sock(&sktable[n]);
	sock_enqueue(&so_free, &sktable[n]);
    }
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_sock(struct sock *soptr)
{
    memset(soptr,0,sizeof(struct sock));
    return;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock_queue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_sock_queue(struct sockqueue *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbygvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getsockbygvport(u_int16_t gvport)
{
    return sk_gvport[gvport];
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * bind_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *bind_gvport(struct sock *sk, u_int16_t gvport)
{
    if ( sk_gvport[gvport] == NULL )
    {
	sk_gvport[gvport] = sk;
	sk->so_local_gvport = gvport;
	return sk;
    }
    return NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * bind_free_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *bind_free_gvport(struct sock *sk)
{
    int i = 1;
    while (sk_gvport[i] != NULL && i++ < sizeof(u_int16_t) );
    if ( i == sizeof(u_int16_t) )
	return NULL; 

    sk_gvport[i] = sk;
    sk->so_local_gvport = (u_int16_t) i;
    return sk;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * bind_free_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *close_gvport_sk (struct sock *sk)
{
    sk_gvport[sk->so_local_gvport] = NULL;
    sk->so_local_gvport = NO_GVPORT;
    return sk;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * close_gvport_gvport 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *close_gvport_gvport(u_int16_t gvport)
{
    struct sock *sk = sk_gvport[gvport];
    if ( sk != NULL )
	return close_gvport_sk(sk);
    return NULL;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbysodata(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getsockbysodata(int so_data)
{
    struct sock *ptr;
    
    ptr = so_used.head;
    while (ptr != NULL)
    {
	if (ptr->so_lodata == so_data)
	    return ptr;
	ptr = ptr->so_next;
    }
    return NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbysoctrl(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getsockbysoctrl(int so_ctrl)
{
    struct sock *ptr;

    ptr = so_used.head;
    while (ptr != NULL)
    {
	if (ptr->so_loctrl == so_ctrl)
	    return ptr;
	ptr = ptr->so_next;
    }
    return NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * setusedsock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void setusedsock(struct sock *sk)
{
    sock_enqueue(&so_used, sk);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getfreesock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getfreesock(void)
{
    return sock_dequeue(&so_free);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_dequeue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *sock_dequeue(struct sockqueue *q)
{
    struct sock *so;
    
    so = q->head;
    if (so != NULL) {
	q->head = so->so_next;
	if (q->head == NULL)
	    q->tail = NULL;
	q->size--;
	so->so_next = NULL;
    }
    return so;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_enqueue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void sock_enqueue (struct sockqueue *q, struct sock *soptr)
{
    soptr->so_next = NULL;
    if (q->tail == NULL && q->head == NULL)
        q->head = soptr;
    else
        q->tail->so_next = soptr;	
    q->tail = soptr;
    q->size++;
}
