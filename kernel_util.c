#include "sock.h"
#include "itc.h"
#include "mbuff.h"
#include "defs.h"
#include "mbuff_queue.h"


/*
    Algortimo para enviar Datos
    
    init_msg_queue(cq);

    mb = mbuff_dequeue(sk->wmemq);   
    mb = prepare_txmb(sk,mb,DATA);
    msgptr = mbtomsg_carrier(mb,0);

*/



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




struct msg *mbtomsg_carrier (struct mbuff *mb, int discard)
{
    struct msg *mptr;

    mptr = alloc_msg_locking();
    if (mptr) {
	mptr->type    = MSG_MBUFF_CARRIER;
	mptr->discard = discard;
	mptr->mb.mbp  = mb;
	mptr->discard = discard;
    }
    return mptr;
}

size_t mbqtomsgq_carrier (struct msg_queue *msg, struct mb_queue *mbuff, int discard)
{
    struct msg   *msgptr;
    struct mbuff *mbptr;
    size_t ret = 0;
    init_msg_queue(msg);

    while ( (mbptr = mbuff_dequeue(mbuff)) != NULL ) {
	msgptr = alloc_msg_locking();
	if (msgptr) {
	    msgptr->type    = MSG_MBUFF_CARRIER;
	    msgptr->discard = discard;
	    msgptr->mb.mbp  = mbptr;
	    msg_enqueue(msg, msgptr);
	    ret++;
	}
	else
	    break;
    }
    return ret;
}