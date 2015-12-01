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

#include "sock.h"
#include "itc.h"
#include "mbuff.h"
#include "defs.h"
#include "heap.h"
#include "util.h"
#include "mbuff_queue.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "glo.h"

/*
 * This is the JSON message when the connection is established with the api
 */
#define JSON_TEMPLATE "{\"Gaver\":\"%s\",\"Status\":\"%d\",\"Reason\":\"%s\"}"
#define JSON_BUFFER   512

/*======================================================================================*
 * do_collect_mbuff_from_sk()								*
 *======================================================================================*/
PRIVATE void do_collect_mbuff_from_sk (struct sock *sk, struct msg_queue *tx, struct msg_queue *txctrl);

/*======================================================================================*
 * do_update_tokens_sock()								*
 *======================================================================================*/
PRIVATE void do_update_tokens_sock (struct sock *sk, u_int64_t times);

/*======================================================================================*
 * sendall()										*
 *======================================================================================*/
PRIVATE ssize_t sendall (int sd, void *buff, size_t len);

/*======================================================================================*
 * send_status()									*
 *======================================================================================*/
PRIVATE int send_status(int sd, int status, char *reason);



/*======================================================================================*
 * prepare_txmb()									*
 *======================================================================================*/
struct mbuff *prepare_txmb (struct sock *sk, struct mbuff *mb, u_int8_t type)
{
    /*
     *	Fill the outside Addr
     */
    mb->m_outside_addr.sin_family 	= AF_INET;
    mb->m_outside_addr.sin_port		= sk->so_host_port;
    mb->m_outside_addr.sin_addr.s_addr	= sk->so_host_addr.s_addr; 

    /*
     * Fill Gaver Header
     */
    mb->m_hdr.src_port		= sk->so_local_gvport; 				
    mb->m_hdr.dst_port		= sk->so_host_gvport;
    mb->m_hdr.payload_len	= mb->m_datalen;
    mb->m_hdr.version		= GAVER_PROTOCOL_VERSION;
    mb->m_hdr.type		= type;

    if (type)	/* Data Type is 0x00 */ 
	mb->m_hdr.seq	= sk->so_cseq_out++;
    else
	mb->m_hdr.seq	= sk->so_dseq_out++;
    
    mb->m_hdrlen = sizeof(struct gvhdr);

    return mb;
}

/*======================================================================================*
 * mbtomsg_carrier()									*
 *======================================================================================*/
struct msg *mbtomsg_carrier (struct mbuff *mb, int discard)
{
    struct msg *mptr;

    mptr = alloc_msg_locking();
    if (mptr) {
	mptr->msg_type = MSG_MBUFF_CARRIER;
	mptr->discard  = discard;
	mptr->mb.mbp   = mb;
	mptr->discard  = discard;
    }
    return mptr;
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

	    nsk->so_dseq_out	 = START_DATASEQ;
	    nsk->so_cseq_out	 = START_CTRLSEQ;

	    nsk->so_resyn	 = 0;
	    nsk->so_avtok	 = 0;
	    
    	    nsk->so_retok	 = getreftime(speed, mtu);	/* Cantidad de tokens a actualizar */

	    nsk->so_mtu		 = mtu;				/* Global MTU */
	    nsk->so_speed	 = speed;			/* Configured Speed */

	    nsk->so_cnt_req	 = NULL;			/* Connection Request */
	    
	    /* Global */
	    free_bps 		-= speed;			/* Update available speed */

	    init_mbuff_queue(&(nsk->so_sentq));
	    init_mbuff_queue(&(nsk->so_wmemq));
	    init_mbuff_queue(&(nsk->so_rmemq));

	    nsk->so_dseq_exp	 = 0;				/* Data seq expected    */
	    nsk->so_cseq_exp	 = 0;				/* Control seq expected */


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
    
    return;
}

/*======================================================================================*
 * do_update_tokens()									*
 *======================================================================================*/
void do_update_tokens (struct sockqueue *sk, u_int64_t times)
{
    struct sockqueue tmp;
    struct sock *skp;
    
    init_sock_queue(&tmp);
        
    while ( (skp = sock_dequeue(sk)) ) {
	if (skp->so_state == GV_ESTABLISHED) 
	    do_update_tokens_sock(skp,times);
	sock_enqueue(&tmp,skp);
    }
    sk->size = tmp.size;
    sk->head = tmp.head;
    sk->tail = tmp.tail;
    return;
}

/*======================================================================================*
 * do_collect_mbuff_to_transmit()							*
 *======================================================================================*/
void do_collect_mbuff_to_transmit (struct sockqueue *sk, struct msg_queue *tx , struct msg_queue *txctrl)
{
    struct sockqueue tmp;
    struct sock *skp;
    
    init_sock_queue(&tmp);

    while ( (skp = sock_dequeue(sk)) ) {
	if (skp->so_state == GV_ESTABLISHED) 
	    do_collect_mbuff_from_sk(skp, tx, txctrl);
	sock_enqueue(&tmp, skp);
    }
    sk->size = tmp.size;
    sk->head = tmp.head;
    sk->tail = tmp.tail;
    return;
}


/*======================================================================================*
 * do_collect_mbuff_from_sk()								*
 *======================================================================================*/
void do_collect_mbuff_from_sk (struct sock *sk, struct msg_queue *tx, struct msg_queue *txctrl)
{
    struct msg *mptr;
    struct mbuff *mbp;
    double one = 1.0;
    int    avtok = (int) floor(sk->so_avtok);	/* Solamente la parte entera */
    int    syn   = (int) floor(sk->so_resyn);    

    mptr = NULL;
    mbp  = NULL;

    if (syn) {
	/* mbp  = prepare_syn(sk); */
	if (mbp) {
	    mptr = mbtomsg_carrier(mbp,1);
	    if (mptr) {
		msg_enqueue(txctrl, mptr);
		sk->so_resyn = sk->so_resyn - one;	/* Update Syn Time */
		mptr = NULL;
		mbp  = NULL;
	    }
	}
    }

    while (avtok > 0) {
	mbp = mbuff_dequeue(&(sk->so_wmemq));
	if (!mbp)
	    break;
	mbp  = prepare_txmb(sk,mbp,DATA);	/* Fill all fields              */
	mptr = mbtomsg_carrier(mbp,0);		/* Add mbuff to msg carrier     */
	if (!mptr) {
	    /* Push */
	    break;	
	}
	msg_enqueue(tx, mptr);			/* Finaly enqueue to transmit   */
	sk->so_avtok = sk->so_avtok - one;	/* Update Sock available tokens */
	avtok--;				
    }
    return;
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

