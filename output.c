#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include "gaver.h"
#include "mbuff_queue.h"
#include "mbuff.h"
#include "itc.h"
#include "glo.h"


#define MAX_OUTPUT_MSG 10

ssize_t sendmbuff (int sd, void *bufdata, size_t lendata, void *bufhdr, size_t lenhdr, struct sockaddr_in *dst_addr);
ssize_t sendqueue (int ifudp, struct msg_queue *queue, size_t qlen);




void *output (void *arg)
{
    struct msg_queue outputq[3];



    /*
     * All itc signal used for comunicacion must be blocked
     */
    itc_block_signal();

    


}



ssize_t sendqueue (int ifudp, struct msg_queue *queue, size_t qlen)
{
    struct mmsghdr   mmsg[MAX_OUTPUT_MSG];
    struct iovec     io[MAX_OUTPUT_MSG*2];
    struct msg_queue tmpq;
    struct mbuff     *mbptr;
    struct msg	     *msgptr;
    ssize_t output_len;
    ssize_t ret;
    int i, qsz;

    if (qlen != queue->size)
	/*
	 * Error al pasar el parametro
	 */
	return -1;

    if (queue->size == 1) {
	mbptr = queue->head->p_mbuff;
	output_len = sendmbuff(ifudp, 
		        mbptr->m_payload, 
			mbptr->m_datalen, 
			&mbptr->m_hdr,
			mbptr->m_hdrlen,
			&mbptr->m_outside_addr);
    }
    else {
	if (queue->size > MAX_OUTPUT_MSG)
	    /*
             * Que no supere el max output
             */
	    return -1;
	else {
	    qsz = queue->size;

	    tmpq.head = NULL;
	    tmpq.tail = NULL;
	    tmpq.size = 0;
	
	    for ( i = 0; i <= qsz -1 && i <= MAX_OUTPUT_MSG -1; i++ ) {
	    /*	struct msghdr {
             *	    void         *msg_name;        optional address 
             *	    socklen_t     msg_namelen;     size of address 
             *	    struct iovec *msg_iov;         scatter/gather array 
             *	    size_t        msg_iovlen;      # elements in msg_iov
             *	    void         *msg_control;     ancillary data, see below 
             *	    size_t        msg_controllen;  ancillary data buffer len 
             *	    int           msg_flags;       flags on received message 
             *	};
	     */
		msgptr = msg_dequeue(queue);

		io[i*2].iov_base		= &msgptr->p_mbuff->m_hdr;
		io[i*2].iov_len			= msgptr->p_mbuff->m_hdrlen;
		io[(i*2)+1].iov_base		= msgptr->p_mbuff->m_payload;
		io[(i*2)+1].iov_len		= msgptr->p_mbuff->m_datalen;
		mmsg[i].msg_hdr.msg_name	= &msgptr->p_mbuff->m_outside_addr;
		mmsg[i].msg_hdr.msg_namelen	= sizeof(struct sockaddr_in);
		mmsg[i].msg_hdr.msg_iov		= &io[i*2];
		mmsg[i].msg_hdr.msg_iovlen	= 2;
		mmsg[i].msg_hdr.msg_control	= NULL;
		mmsg[i].msg_hdr.msg_controllen	= 0;
		mmsg[i].msg_hdr.msg_flags	= 0;
		mmsg[i].msg_len			= 0;

		msg_enqueue(&tmpq, msgptr);
	    }
	    ret = sendmmsg(ifudp, 
			   mmsg,
			   qsz,
			   0);

	    msgqcat(queue, &tmpq);
	    if (ret != -1) {
		output_len = 0;
		for ( i = 0; i <= qsz -1 && i <= MAX_OUTPUT_MSG -1; i++ )
		    output_len += mmsg[i].msg_len;
	    }
	    else
		output_len = ret;
	}

    }
    return output_len;

}



#define SENDTO_MAX_BUFFER 2048
ssize_t sendmbuff (int sd, void *bufdata, size_t lendata, void *bufhdr, size_t lenhdr,
			    struct sockaddr_in *dst_addr)
{
    ssize_t ret;
    socklen_t alen = sizeof(struct sockaddr_in);
#ifdef USE_SENDTO
    unsigned char buf[SENDTO_MAX_BUFFER];
#else
    struct msghdr msg;
    struct iovec  io[2];
#endif

#ifdef USE_SENDTO

    if (lenhdr + lendata > SENDTO_MAX_BUFFER)
	/*
         * De momento falla de manera sileciosa
         */
	return -1;

    if (lenhdr > 0 && bufhdr != NULL )
	memmove(buf, bufhdr, lenhdr);
    else
	lenhdr = 0;
    memmove(&buf[lenhdr], bufdata, lendata)
    ret = sendto(sd,
		 buf,
		 lenhdr + lendata,
		 0,
		 (const struct sockaddr *) dst_addr,
		 alen);
#else
    memset(&msg, 0, sizeof(struct msghdr));
    if (lenhdr > 0 && bufhdr != NULL)
    {
	io[0].iov_base = bufhdr;
        io[0].iov_len  = lenhdr;
	io[1].iov_base = bufdata;
	io[1].iov_len  = lendata;
	msg.msg_iov      = io;
	msg.msg_iovlen   = 2;
    }
    else
    {
	io[0].iov_base = bufdata;
	io[0].iov_len  = lendata;
	msg.msg_iov      = io;
	msg.msg_iovlen   = 1;
    }
    msg.msg_name     = dst_addr;
    msg.msg_namelen  = alen;
    
    ret = sendmsg(sd,
		  &msg,
		  0);
#endif
    return ret;
}
