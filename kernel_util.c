#include "sock.h"
#include "mbuff.h"
#include "defs.h"
#include "mbuff_queue.h"



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


