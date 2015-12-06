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

#ifndef _SOCK_H
#define _SOCK_H

#include "mbuff.h"
#include "types.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * CONTROL SOCKET STATE									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
enum {
    CTRL_NONE = 1,
    CTRL_CONNECT_REQUEST,
    CTRL_WAITING_CONNECT,
    CTRL_WAITING_ACCEPT_REPLY,
    CTRL_ESTABLISHED
};

#define DATA_IO_NONE		0x00	/* 0000 */
#define DATA_IO_WAITING_MEM	0x04	/* 0100 */
#define DATA_IO_READ_PENDING	0x01	/* 0001 */
#define DATA_IO_WRITE_PENDING	0x02	/* 0010 */
#define DATA_IO_RW_PENDING	0x03	/* 0011 = IO_READ_PENDING | IO_WRITE_PENDING */

#define MAX_SOCKETS	  256
#define NO_GVPORT         0x0000

typedef struct in_addr in_addr;


struct sock {
    int 	so_lodata;
    int 	so_lodata_state;	/* DATAIO_READ_PENDING | DATAIO_WRITE_PENDING */
    int 	so_loctrl;
    int 	so_loctrl_state;
    in_addr	so_host_addr;
    u_int16_t	so_host_port;
    u_int16_t   so_host_gvport;
    u_int16_t	so_local_gvport;
    int		so_state;


    int		so_mtu;			/* Mtu for the socket	*/
    u_int64_t   so_speed;

    double      so_resyn;
    double	so_retok;
    double	so_avtok;

    u_int8_t	so_capwin;		/* Congestion Avoidance Percent Window */
    size_t	so_host_win;		/* Ventana declarada por el receptor   */


    u_int64_t	so_dseq_out;		/* Data seq out         */
    u_int64_t	so_cseq_out;		/* Contrl seq out       */
    u_int64_t	so_dseq_exp;		/* Data seq expected    */
    u_int64_t   so_cseq_exp;		/* Control seq expected */


    size_t wmem_size;			/* Write buffer size    */
    size_t rmem_size;			/* Read  buffer size    */
    

    struct mbuff *so_cnt_req;		/* Connection Req	*/

    struct mb_queue so_wmemq;
    struct mb_queue so_rmemq;


    struct msg_queue so_data_sent;		/* Mbuff data sent with carrier */
    struct msg_queue so_ctrl_sent;		/* Mbuff ctrl sent with carrier */

    /*
    struct mb_queue roooq;	Reception Out Of Order 

    struct seq_lost_queue lostq;	 Queue of lost Trasport Data Unit (mbuff)

    */

    struct sock *so_next;
};

struct sockqueue {
    size_t size;
    struct sock *head;
    struct sock *tail;
};


#ifdef _SOCK_CODE
#undef EXTERN
#define EXTERN
#endif
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock_table(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN void init_sock_table(void);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN void init_sock(struct sock *soptr);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock_queue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN void init_sock_queue(struct sockqueue *q);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbygvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *getsockbygvport(u_int16_t gvport);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * bind_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *bind_gvport(struct sock *sk, u_int16_t gvport);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * bind_free_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *bind_free_gvport(struct sock *sk);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * bind_free_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *close_gvport_sk (struct sock *sk);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * close_gvport_gvport 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *close_gvport_gvport(u_int16_t gvport);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbysodata(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *getsockbysodata(int so_data);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbysoctrl(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *getsockbysoctrl(int so_ctrl);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * setusedsock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN void setusedsock(struct sock *sk);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getfreesock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *getfreesock(void);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_dequeue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN struct sock *sock_dequeue(struct sockqueue *q);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_enqueue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
EXTERN void sock_enqueue (struct sockqueue *q, struct sock *soptr);

#endif
