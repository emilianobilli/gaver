#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

 /* According to POSIX.1-2001 */
#include <sys/select.h>

 /* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "gaver.h"
#include "mbuff_queue.h"
#include "mbuff.h"
#include "itc.h"
#include "glo.h"

#define MAX_OUTPUT_MSG 10
#define TIMER_EXPIRATION 0x01
#define ITC_EVENT	 0x02

static ssize_t sendmbuff (int sd, void *bufdata, size_t lendata, void *bufhdr, size_t lenhdr, struct sockaddr_in *dst_addr);
static ssize_t sendqueue (int ifudp, struct msg_queue *queue, size_t qlen);
static int wait_event(int timer_event, int itc_event, u_int8_t *event);



int wait_event(int timer_event, int itc_event, u_int8_t *event)
{
    fd_set event;
    int max, ret;    

    FD_ZERO(&event);
    FD_SET(output_timer,&event);
    FD_SET(sefd, &event);
    
    max = (timer_event > itc_event) ? timer_event : itc_event;
    
    while(1) {
	ret = select(max+1, &event, NULL, NULL, NULL);
	if (ret == -1) {
	    if (errno == EINTR)
		continue;
	    else
		return -1;
	}
	else
	    break;
    }
    *event = 0;
    if (FD_ISSET(timer_event, &event))
	*event |= TIMER_EXPIRATION;
    if (FD_ISSET(itc_event, &event))
	*event |= ITC_EVENT;
    
    return ret;
}


void *output (void *arg)
{
    struct msg_queue outputq[3];
    struct msg_queue txq;
    u_int8_t  event;		/* ITC Event	    */
    u_int64_t exp;		/* Expiration Times */
    int msg_pending		/* Flag if Event arrives */
    
    
    /*
     * All itc signal used for comunicacion MUST be blocked
     */
    itc_block_signal();

    init_msg_queue(&outpuq[PRIO_CTR_QUEUE]);
    init_msg_queue(&outpuq[PRIO_RET_QUEUE]);
    init_msg_queue(&outpuq[PRIO_NOR_QUEUE]);
    init_msg_queue(&txq);

    msg_pending = 0;

    while(1) 
    {
	ret = wait_event(output_timer, itc_event, &event);

	if (ret == -1) 
	{
	    /*
	     * Panic()
    	     */
	    pthread_exit(&ret);
	}
	if (event & TIMER_EXPIRATION) 
	{
	    ret = gettimerexp(output_timer, &exp);
	    if (ret == -1) 
	    {
		/*
		 * Panic()
		 */
		pthread_exit(&ret);
	    }

	}
	if (event & ITC_EVENT) {

    
	}
    }


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

    if (queue->size == 1) 
    {
	mbptr = queue->head->p_mbuff;
	output_len = sendmbuff(ifudp, 
		        mbptr->m_payload, 
			mbptr->m_datalen, 
			&(mbptr->m_hdr),
			mbptr->m_hdrlen,
			&(mbptr->m_outside_addr));
	if (output_len == mbptr->m_datalen + mbptr->m_hdrlen)
	{
	    msgptr = msg_dequeue(queue);
	    if (msgptr->discard == DISCARD_TRUE)
		free_mbuff_locking(msgptr->p_mbuff);
	    free_msg_locking(msgptr);
	}
    }
    else 
    {
	if (queue->size > MAX_OUTPUT_MSG)
	    /*
             * Que no supere el max output
             */
	    return -1;
	else 
	{
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

		io[i*2].iov_base		= &(msgptr->p_mbuff->m_hdr);
		io[i*2].iov_len			= msgptr->p_mbuff->m_hdrlen;
		io[(i*2)+1].iov_base		= msgptr->p_mbuff->m_payload;
		io[(i*2)+1].iov_len		= msgptr->p_mbuff->m_datalen;
		mmsg[i].msg_hdr.msg_name	= &(msgptr->p_mbuff->m_outside_addr);
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

	    if (ret != -1) 
	    {
		output_len = 0;
		for ( i = 0; i <= qsz -1 && i <= MAX_OUTPUT_MSG -1; i++ ) 
		{
		    msgptr = msg_dequeue(&tmpq);
		    if (mmsg[i].msg_len == msgptr->p_mbuff->m_hdrlen + msgptr->p_mbuff->m_datalen) 
		    {
			/*
			 * Si los datos enviados concuerdan
			 *  	- Se incrementa output_len
			 *	- Le libera msg
			 *	- Se libera mbuff si corresponde
			 */
			output_len += mmsg[i].msg_len;
			if (msgptr->discard == DISCARD_TRUE)
			    free_mbuff_locking(msgptr->p_mbuff);
			msgptr->p_mbuff = NULL;
			free_msg_locking(msgptr);
		    }
		    else if (mmsg[i].msg_len == 0)
			msg_enqueue(queue, msgptr);
		}
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
