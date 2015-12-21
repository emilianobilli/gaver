#include "mbuff.h"
#include "heap.h"
#include "gaver.h"
#include "sock.h"
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>

#define _KERNEL_TX_CODE
#include "kernel_tx.h"

/*======================================================================================*
 * clone_msg_carrier()									*
 *======================================================================================*/
struct msg *clone_msg_carrier (struct msg *m)
{
    struct mbuff *mbuff;
    struct msg   *msg = NULL;

    if (m->msg_type == MSG_MBUFF_CARRIER) {
	mbuff = alloc_mbuff_locking();
        if (mbuff) {
	    memcpy(mbuff, m->mb.mbp, sizeof(struct mbuff));
	    msg = alloc_msg_locking();
	    if (msg) {
		memcpy(msg,m, sizeof(struct msg));
		msg->type.carrier.discard = DISCARD_TRUE;
		msg->mb.mbp = mbuff;	    
	    }
	    else 
		free_mbuff_locking(mbuff);

	}
    }
    return msg;
}


/*======================================================================================*
 * prepare_connect()									*
 *======================================================================================*/
struct msg *prepare_connect (struct sock *sk)
{
    struct msg	     *msg;
    struct gvconn    *connect;
    struct mbuff     *mb;

    mb = alloc_mbuff_locking();
    msg = NULL;
    if (mb) {
	mb->m_datalen = sizeof(struct gvconn);
	mb->m_need_ts = DO_TS;

	/* OJO!!!! Con el network byte order */

	connect = (struct gvconn *) mb->m_payload;
    	connect->data_seq	= sk->so_dseq_out;
	connect->speed		= sk->so_speed;
	connect->recv_window	= 0;	/* Ojo!!!, es necesario cambiar el recv_window */
	connect->mtu		= sk->so_mtu;
	connect->speed_type	= SPEED_FAIR;

	sk->so_conn_attempts	= 1;
    
	msg = prepare_txmsg(sk,mb,CONNECT,sk->so_cseq_out,DISCARD_FALSE);
	if (!msg)
	    free_mbuff_locking(mb);
	else
	    sk->so_cseq_out++;
    }
    return msg;
}

/*======================================================================================*
 * prepare_accept()									*
 *======================================================================================*/
struct msg *prepare_accept (struct sock *sk, struct mbuff *creq, int ctrl_ack)
{
    struct msg		*msg;
    struct gvctrlack    *control;
    struct gvconn	*accept;
    struct mbuff	*mb;
    u_int8_t		type;

    mb  = alloc_mbuff_locking();
    msg = NULL;

    mb->m_need_ts = DO_TS;

    if (mb) {
	if (ctrl_ack) {
	    type = CTRL_ACK | ACCEPT;
	    mb->m_datalen = sizeof(struct gvctrlack) + sizeof(struct gvconn);
	    control = (struct gvctrlack *) mb->m_payload;
    	    control->ctrl_seq_ack	= creq->m_hdr.seq;
	    control->peer_ts_sec	= creq->m_hdr.txts_sec;
	    control->peer_ts_nsec	= creq->m_hdr.txts_nsec;
	    control->rx_ts_sec		= creq->m_input_ts[0];
	    control->rx_ts_nsec		= creq->m_input_ts[1];
	    accept 			= (struct gvconn *)&(mb->m_payload)[sizeof(struct gvctrlack)]; 
	}
	else {
	    type = ACCEPT;
	    mb->m_datalen = sizeof(struct gvconn);
	    accept			= (struct gvconn *)&(mb->m_payload)[0];
	}

	accept->data_seq	= sk->so_dseq_out;
	accept->speed		= sk->so_speed;
	accept->recv_window	= 0;	/* Ojo Modificar Speed */
	accept->mtu		= sk->so_mtu;
	accept->speed_type	= SPEED_FAIR;


	msg = prepare_txmsg(sk,mb,type,sk->so_cseq_out,DISCARD_FALSE);
	if (!msg)
	    free_mbuff_locking(mb);
	else
	    sk->so_cseq_out++;
    }
    return msg;
}

/*======================================================================================*
 * prepare_syn()									*
 *======================================================================================*/
struct msg *prepare_syn(struct sock *sk)
{
    return NULL;
}


/*======================================================================================*
 * prepare_ctrl_ack()									*
 *======================================================================================*/
struct msg *prepare_ctrl_ack (struct sock *sk, struct mbuff *m)
{
    struct msg	     	*msg;
    struct gvctrlack    *control;

    struct mbuff     *mb;
    mb = alloc_mbuff_locking();
    msg = NULL;

    /*
    struct gv_ctrl_ack {
	u_int64_t ctrl_seq_ack;
        u_int64_t peer_ts_sec;
        u_int64_t peer_ts_nsec;
        u_int64_t rx_ts_sec;
        u_int64_t rx_ts_nsec;
    };
    */
    if (mb) {
	mb->m_datalen = sizeof(struct gvctrlack);
	mb->m_need_ts = DO_TS;

	/* OJO!!!! Con el network byte order */

	control = (struct gvctrlack *) mb->m_payload;
    	control->ctrl_seq_ack	= m->m_hdr.seq;
	control->peer_ts_sec	= m->m_hdr.txts_sec;
	control->peer_ts_nsec	= m->m_hdr.txts_nsec;
	control->rx_ts_sec	= m->m_input_ts[0];
	control->rx_ts_nsec	= m->m_input_ts[1];

	if (sk)    
	    msg = prepare_txmsg(sk,mb,CTRL_ACK,0,DISCARD_TRUE);
	else
	    msg = prepare_txmsg_from_mbuff(m,mb,CTRL_ACK,0,DISCARD_TRUE);

	if (!msg)
	    free_mbuff_locking(mb);
    }
    return msg;
}


/*======================================================================================*
 * prepare_txmsg_from_mbuff()								*
 *======================================================================================*/
struct msg *prepare_txmsg_from_mbuff (struct mbuff *rx, struct mbuff *mb, u_int8_t type, u_int64_t seq, int discard)
{
    struct msg *mptr;

    mptr = alloc_msg_locking();
    if (mptr) {
    
	mptr->msg_type = MSG_MBUFF_CARRIER;
	mptr->type.carrier.discard	= discard;
	mptr->type.carrier.type		= type;
	mptr->type.carrier.seq		= seq;
	mptr->mb.mbp			= mb;

	/*
         *	Fill the outside Addr
	 */
	mb->m_outside_addr.sin_family 		= AF_INET;
        mb->m_outside_addr.sin_port		= rx->m_outside_addr.sin_port;
        mb->m_outside_addr.sin_addr.s_addr	= rx->m_outside_addr.sin_addr.s_addr;

	/*
	 * 	Fill Gaver Header
         */
	mb->m_hdr.src_port		= rx->m_hdr.dst_port; 				
        mb->m_hdr.dst_port		= rx->m_hdr.src_port;
        mb->m_hdr.payload_len		= mb->m_datalen;
        mb->m_hdr.version		= GAVER_PROTOCOL_VERSION;
        mb->m_hdr.type			= type;
	mb->m_hdr.seq			= seq;

	mb->m_next = NULL;
	
    	mb->m_hdrlen = sizeof(struct gvhdr);
    }
    return mptr;
}



/*======================================================================================*
 * prepare_txmsg()									*
 *======================================================================================*/
struct msg *prepare_txmsg (struct sock *sk, struct mbuff *mb, u_int8_t type, u_int64_t seq, int discard)
{
    struct msg *mptr;

    mptr = alloc_msg_locking();
    if (mptr) {
    
	mptr->msg_type = MSG_MBUFF_CARRIER;
	mptr->type.carrier.discard	= discard;
	mptr->type.carrier.type		= type;
	mptr->type.carrier.seq		= seq;
	mptr->mb.mbp			= mb;

	/*
         *	Fill the outside Addr
	 */
	mb->m_outside_addr.sin_family 		= AF_INET;
        mb->m_outside_addr.sin_port		= sk->so_host_port;
        mb->m_outside_addr.sin_addr.s_addr	= sk->so_host_addr.s_addr; 

	/*
	 * 	Fill Gaver Header
         */
	mb->m_hdr.src_port		= sk->so_local_gvport; 				
        mb->m_hdr.dst_port		= sk->so_host_gvport;
        mb->m_hdr.payload_len		= mb->m_datalen;
        mb->m_hdr.version		= GAVER_PROTOCOL_VERSION;
        mb->m_hdr.type			= type;
	mb->m_hdr.seq			= seq;

	mb->m_next = NULL;
	
    	mb->m_hdrlen = sizeof(struct gvhdr);
    }
    return mptr;
}