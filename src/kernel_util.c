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
#include "gaver.h"
#include "timers.h"
#include "kernel_api.h"
#include "apitypes.h"
#include "kernel_defs.h"
#include "mbuff_queue.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "glo.h"
#define _KERNEL_UTIL_CODE
#include "kernel_util.h"
#include "kernel_tx.h"
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
 * do_process_input()									*
 *======================================================================================*/
PRIVATE void do_process_input (struct sock *sk, struct mbuff *mbuff, struct msg_queue *ctrq);

/*======================================================================================*
 * do_distribute_sent_msg()								*
 *======================================================================================*/
PRIVATE void do_distribute_sent_msg (struct sock *sk, struct msg *m);


/*======================================================================================*
 * do_check_source()									*
 *======================================================================================*/
PRIVATE int do_check_source (struct sock *sk, struct mbuff *mbuff);



/*======================================================================================*
 * get_seq()										*
 *======================================================================================*/
u_int64_t get_seq(struct mbuff *m)
{
    return m->m_hdr.seq;
}

u_int64_t get_ctrl_ack_seq(struct mbuff *m)
{
    struct gvctrlack *ctrl_ack = (struct gvctrlack *) m->m_payload;

    return ctrl_ack->ctrl_seq_ack;
}

/*======================================================================================*
 * do_accept_connection()								*
 *======================================================================================*/
struct msg *do_accept_connection (struct sock *sk, struct mbuff *conn_req, int ctrl_ack)
{
    struct sockaddr_in *addr;
    struct gvconn      *connect;

    connect = (struct gvconn   *)   &(conn_req->m_payload);
    addr = (struct sockaddr_in *)   &(conn_req->m_outside_addr);

    sk->so_host_addr.s_addr = addr->sin_addr.s_addr;
    sk->so_host_port        = addr->sin_port;
    sk->so_host_gvport      = get_source_port(conn_req); 
    
    sk->so_commited_mtu   = (connect->mtu   < sk->so_mtu)   ? connect->mtu   : sk->so_mtu;
    sk->so_commited_speed = (connect->speed < sk->so_speed) ? connect->speed : sk->so_speed;
    sk->so_dseq_exp = connect->data_seq;
    sk->so_cseq_exp = get_seq(conn_req) + 1;
    sk->so_retok    = getreftime(sk->so_commited_speed, sk->so_commited_mtu);
    sk->so_conn_attempts++;

    return prepare_accept(sk, conn_req, ctrl_ack);
}

void do_commit_connection (struct sock *sk, struct mbuff *m, off_t offset)
{
    struct gvconn *accept;

    accept = (struct gvconn   *)   &(m->m_payload)[offset];

    sk->so_commited_mtu   = accept->mtu;
    sk->so_commited_speed = accept->speed;
    sk->so_dseq_exp = accept->data_seq;
    sk->so_cseq_exp = get_seq(m) + 1;
    sk->so_retok   = getreftime(sk->so_commited_speed, sk->so_commited_mtu);

    return;
}


/*======================================================================================*
 * get_destination_port()								*
 *======================================================================================*/
u_int16_t get_destination_port(struct mbuff *m)
{
    return m->m_hdr.dst_port;
}
/*======================================================================================*
 * get_source_port_from_msg()							*
 *======================================================================================*/
u_int16_t get_source_port_from_msg(struct msg *m)
{
    return get_source_port(m->mb.mbp);
}
/*======================================================================================*
 * get_source_port()									*
 *======================================================================================*/
u_int16_t get_source_port(struct mbuff *m)
{
    return m->m_hdr.src_port;
}
/*======================================================================================*
 * get_type_from_msg()									*
 *======================================================================================*/
u_int8_t get_type_from_msg(struct msg *m)
{
    return m->type.carrier.type;
}
/*======================================================================================*
 * get_type()										*
 *======================================================================================*/
u_int8_t get_type(struct mbuff *m)
{
    return m->m_hdr.type;
}

/*======================================================================================*
 * do_distribute_sent_msg()								*
 *======================================================================================*/
void do_distribute_sent_msg (struct sock *sk, struct msg *m)
{
    if (get_type_from_msg(m))
        /* Is Control */
        msg_enqueue(&(sk->so_ctrl_sent),m);
    else 
        /* Is Data */
        msg_enqueue(&(sk->so_data_sent),m);

    return;
}

/*======================================================================================*
 * do_process_sent_msg()								*
 *======================================================================================*/
void do_process_sent_msg (struct msg_queue *sentq)
{
    struct msg *mptr;
    struct sock *sk;

    while ( (mptr = msg_dequeue(sentq)) != NULL ) 
    {
	sk = sk_gvport[get_source_port_from_msg(mptr)];
	if (sk) {
	    if (mptr->type.carrier.sent_result == SENT_ERROR) {
		do_socket_error_response(sk, mptr->type.carrier.sent_error);
		free_mbuff_locking(mptr->mb.mbp);
		free_msg_locking(mptr);
	    }
	    else 
		do_distribute_sent_msg(sk,mptr);
	}    
    }
}


/*======================================================================================*
 * do_process_input_bulk()								*
 *======================================================================================*/
void do_process_input_bulk (struct msg_queue *inputq, struct msg_queue *ctrq)
{
    struct sock  *sk;
    struct msg   *m;
    struct mbuff *mb;
    u_int16_t dst_port;

    while ( (m = msg_dequeue(inputq)) != NULL )
    {
	mb = m->mb.mbp;
	free_msg_locking(m);
	dst_port = get_destination_port(mb);

	sk = sk_gvport[dst_port];	/* Global Table */
	if (sk) 
	    do_process_input(sk,mb, ctrq);
	else	
	    /* DROP */
	    free_mbuff_locking(mb);
    }
    return;
}


/*======================================================================================*
 * do_check_source()									*
 *======================================================================================*/
int do_check_source (struct sock *sk, struct mbuff *mbuff)
{
    struct sockaddr_in *addr = &(mbuff->m_outside_addr);
    u_int16_t gv_port = get_source_port(mbuff);
    
    if (sk->so_host_addr.s_addr == addr->sin_addr.s_addr &&
	sk->so_host_port        == addr->sin_port &&
	sk->so_host_gvport      == gv_port )
	return 1;
    return 0;
}


int check_seq (void *void_seq, struct mbuff *mbuff)
{
    u_int64_t *seq = void_seq;

    if (get_seq(mbuff) == *seq)
	return 1;

    return 0;
}


/*======================================================================================*
 * do_rx_ctrl_ack()									*
 *======================================================================================*/
int do_rx_ctrl_ack(struct sock *sk, struct mbuff *mbuff)
{
    struct msg *msg;

    u_int64_t seq      = get_ctrl_ack_seq(mbuff);
    u_int8_t  msg_type = get_type(mbuff);

    if (msg_type & CTRL_ACK) {
	msg = msg_search_custom(&(sk->so_ctrl_sent), &seq, check_seq);
	if (msg) {
	    remove_et(sk,msg->mb.mbp);
	    free_mbuff_locking(msg->mb.mbp);
	    free_msg_locking(msg);
	    return 1;
	}
    }
    return 0;
}	


/*======================================================================================*
 * do_process_input()									*
 *======================================================================================*/
void do_process_input (struct sock *sk, struct mbuff *mbuff, struct msg_queue *ctrq)
{
    u_int8_t msg_type;
    struct msg *ctr_msg;
    struct timespec ts;
    off_t  offset;

    msg_type = get_type(mbuff);

    switch (sk->so_state)
    {
	case GV_ESTABLISHED:
	    break;
	case GV_CONNECT_RCVD:
	    if (msg_type == CONNECT)
		goto drop;

	    break;
    	case GV_CONNECT_SENT:
	    offset = 0;
	    if (do_check_source(sk,mbuff)) {
		if (msg_type & CTRL_ACK) {
		    do_rx_ctrl_ack(sk,mbuff);
		    offset = sizeof(struct gvctrlack);
		    /*
		     * Ojo, hay que chequear que se haga el ctrl_ack primero
		     */
		}
		if (msg_type & ACCEPT ) 
		{
		    do_commit_connection(sk,mbuff,offset);
		    ctr_msg = prepare_ctrl_ack(sk,mbuff);
		    if (ctr_msg) {
			msg_enqueue(ctrq, ctr_msg);
		    }    
		    sk->so_state = GV_ESTABLISHED;
		    do_socket_establish_connection(sk);
		}
	    }
	    break;


	case GV_CLOSE:
	    goto drop;
	    break;

	case GV_LISTEN:
	    if (msg_type & CONNECT)
	    {
		sk->so_state 		= GV_CONNECT_RCVD;
		sk->so_conn_req		= mbuff;
	    
		if (sk->so_loctrl_state == CTRL_ACCEPT_REQUEST) {
		    ctr_msg = do_accept_connection(sk,mbuff,1);
		    if (ctr_msg) {
			msg_enqueue(ctrq, ctr_msg);
			ts.tv_sec  = START_TIMEOUT_SEC;
			ts.tv_nsec = START_TIMEOUT_NSEC;
			/* OJO!!!! No estoy combrobando retorno */
			register_et(sk,ctr_msg->mb.mbp, &ts);
			sk->so_conn_req	    = NULL;
			sk->so_state	    = GV_ACCEPT_SENT;
		    }
		    else {
			sk->so_state    = GV_LISTEN;
			sk->so_conn_req = NULL;
		    }
		}
		else {
		    /* Si llega a esta punto quiere decir que se recibio
		     * un mensaje CONNECT pero de momento la API no 
		     * ejecuto un ACCEPT, lo que se debe hacer es acusar que se recibio
		     * el mensaje
		     */
		    ctr_msg = prepare_ctrl_ack(NULL,mbuff);
		    if (ctr_msg) {
			msg_enqueue(ctrq, ctr_msg);
			goto fin;
		    }    
		}
	    }
	    break;

	case GV_ACCEPT_SENT:
	    if ( (msg_type & CTRL_ACK)  &&
		do_check_source(sk,mbuff) )
		if (do_rx_ctrl_ack(sk,mbuff)) {
		    sk->so_state = GV_ESTABLISHED;
		    do_socket_establish_connection(sk);
		}
	    break;
	case GV_FINISH_SENT:
	case GV_CLOSE_WAIT:
	    break;

	default:
	    goto drop;
	    break;
    }

drop:
    free_mbuff_locking(mbuff);
fin:
    return;
}



/*======================================================================================*
 * do_process_expired()									*
 *======================================================================================*/
void do_process_expired (struct msg_queue *ctrq)
{
    struct sock     *sk;
    struct exptimer *et;
    struct msg      *m;
    struct timespec  ts;
    /*
     * Falta establecer que tipo de timers voy a tener y tambien hay que tener 
     * en cuenta lo que puede suceder de acuerdo a las distintas circunstancias
     */
    while ( (et = get_expired(NULL)) != NULL )
    {
	sk = et->et_sk;
	
	m = msg_search(&(sk->so_ctrl_sent), et->et_mb);
	if (!m) {
	    free_et(et);
	    continue;
	}

	if (sk->so_state == GV_CONNECT_SENT && sk->so_loctrl_state == CTRL_CONNECT_REQUEST ) 
	{
	    if (get_type(et->et_mb) == CONNECT)  
	    {
	    	if (sk->so_conn_attempts < MAX_CONN_ATTEMPTS)
	        {
		    sk->so_conn_attempts++;
		    ts.tv_sec  = START_TIMEOUT_SEC; 
		    ts.tv_nsec = START_TIMEOUT_NSEC;
		    refresh_et(et, &ts);
		    /*
		     * Ojo cambiar todo este algoritmo
		     */
		    msg_enqueue(ctrq, clone_msg_carrier(m));
		    msg_enqueue(&(sk->so_ctrl_sent),m);
		}
		else {
		    do_socket_error_response(sk, ETIMEDOUT);
		    free_et(et);
		}
	    }
	    else {
		do_socket_error_response(sk, EBADE);
		free_et(et);
	    }
	}
	if (sk->so_state == GV_ACCEPT_SENT && sk->so_loctrl_state == CTRL_ACCEPT_REQUEST )
	{
	    if (get_type(et->et_mb) & ACCEPT)
	    {
		if (sk->so_conn_attempts < MAX_CONN_ATTEMPTS)
	        {
		    sk->so_conn_attempts++;
		    ts.tv_sec  = START_TIMEOUT_SEC; 
		    ts.tv_nsec = START_TIMEOUT_NSEC;
		    refresh_et(et, &ts);
		    /*
		     * Ojo cambiar todo este algoritmo
		     */
		    msg_enqueue(ctrq, clone_msg_carrier(m));
		    msg_enqueue(&(sk->so_ctrl_sent),m);
		}
		else {
		    /*
		     * OJO!!! En este caso que se debe hacer
		     */	
		    do_socket_error_response(sk, ETIMEDOUT);
		    free_et(et);
		}
	    }
	}
    }
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

	    memset(nsk->so_data_path, 0, SUN_PATH_SIZE);

	    nsk->so_resyn	 = 0;
	    nsk->so_avtok	 = 0;

	    nsk->so_conn_attempts= 0;
	    
    	    nsk->so_retok	 = getreftime(speed, mtu);	/* Cantidad de tokens a actualizar */

	    nsk->so_commited_mtu = 0;
	    nsk->so_commited_speed = 0;
	    nsk->so_mtu		 = mtu;				/* Global MTU */
	    nsk->so_speed	 = speed;			/* Configured Speed */

	    nsk->so_conn_req	 = NULL;			/* Connection Request */
	    
	    /* Global */
	    free_bps 		-= speed;			/* Update available speed */

	    init_msg_queue(&(nsk->so_data_sent));
	    init_msg_queue(&(nsk->so_ctrl_sent));

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
    sq = sk->so_data_sent.size;		/* Sent Queue  */
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
    int    end   = 0;

    mptr = NULL;
    mbp  = NULL;

    if (syn) {
	mptr  = prepare_syn(sk);
	if (mptr) {
	    msg_enqueue(txctrl, mptr);
	    sk->so_resyn = sk->so_resyn - one;	/* Update Syn Time */
	    mptr = NULL;
	    mbp  = NULL;
	}
    }
    while (avtok > 0 && !end) {
	/*
	 * OJO Con las retransmisiones
	 */
	mbp = mbuff_dequeue(&(sk->so_wmemq));
	if (mbp) {
	    mptr = prepare_txmsg(sk,mbp,DATA, sk->so_dseq_out,DISCARD_FALSE);		/* Fill all fields only         */
	    if (mptr) {
		sk->so_dseq_out++;
		msg_enqueue(tx, mptr);				/* Finaly enqueue to transmit   */
		sk->so_avtok = sk->so_avtok - one;		/* Update Sock available tokens */
		avtok--;
	    }
	    else {
		mbuff_push(&(sk->so_wmemq), mbp);		/* Push In write Queue again    */
	        end = 1;
	    }
	}
	else
	    end = 1;
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

