#define INPUT_CODE
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include "glo.h"
#include "input.h"
#include "input_var.h"
#include "heap.h"
#include "itc.h"
#include "util.h"
#include "mbuff_queue.h"
#include "mbuff.h"
#include "types.h"


PRIVATE ssize_t recvmbuff (int sd, struct mbuff *mbptr);
PRIVATE ssize_t recvdgram (int sd, void *data, size_t datalen, void *hdr, size_t hdrlen, struct sockaddr_in *src_addr);

ssize_t recvmbuff (int sd, struct mbuff *mbptr)
{
    struct timespec ts;
    ssize_t ret ;

    ret = recvdgram(sd,
		    mbptr->m_payload,
		    mtu,		/* Global Var */
		    &(mbptr->m_hdr),
		    sizeof(struct gvhdr),
		    &(mbptr->m_outside_addr));
    
    if (ret != -1) 
    {
	clock_gettime(CLOCK_MONOTONIC, &ts);
	mbptr->m_input_ts[0] = (u_int64_t) ts.tv_sec;
	mbptr->m_input_ts[1] = (u_int64_t) ts.tv_nsec;
    }
    return ret;
}

ssize_t recvdgram (int sd, void *data, size_t datalen, void *hdr, size_t hdrlen, struct sockaddr_in *src_addr)
{
    socklen_t alen = sizeof(struct sockaddr_in);
    struct msghdr msg;
    struct iovec  io[2];
    ssize_t ret;

    io[0].iov_base = hdr;
    io[0].iov_len  = hdrlen;
    io[1].iov_base = data;
    io[1].iov_len  = datalen;

    msg.msg_name       = src_addr;
    msg.msg_namelen    = alen;
    msg.msg_iov	       = &io[0];
    msg.msg_iovlen     = 2;
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags      = 0;

    while(1)
    {
	ret = recvmsg(sd, 
		      &msg,
		      0);
	if ( ret > 0 || (ret == -1 && errno != EINTR) )
	    break;
    }
    return ret;
}



void *input (void *arg)
{
    struct msg_queue rxq;
    struct msg *msgptr; 
    char   *where;
    ssize_t ret;

    /*
     * All itc signal used for comunicacion MUST be blocked
     */
    itc_block_signal();

    init_msg_queue(&rxq);

    msg_recv = 0;		/* Global */
    bytes_recv = 0;		/* Global */

    while(1)
    {
	/*
	 * No comprueba retorno
	 * Pide un solo paquete -- Mejorar --
	 */
	alloc_mbuff_chain(&rxq, 1);
	msgptr = msg_dequeue(&rxq);
	if ( msgptr != NULL ) {
	    ret = recvmbuff(ifudp, msgptr->p_mbuff);
	    if (ret == -1) {
		where = "recvmbuff()";
		goto panic;
	    }
	    /*
	     * Estadisticas
	     */
	    msg_recv ++;
	    bytes_recv +=ret;

	    /*
	     * Lo envia al kernel
	     */
	    msg_enqueue(&rxq, msgptr);
	    itc_writeto(KERNEL_LAYER_THREAD,
			&rxq,
			0);
	}
    }
    pthread_exit(NULL);
panic:
    PANIC(errno, "NETINP_LAYER_THREAD", where);
    return NULL;
}