
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

#define _GNU_SOURCE
#define OUTPUT_CODE
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
#include <errno.h>
#include "output.h"
#include "gaver.h"
#include "mbuff_queue.h"
#include "mbuff.h"
#include "heap.h"
#include "itc.h"
#include "util.h"
#include "glo.h"

PRIVATE ssize_t sendmbuff (int sd, void *bufdata, size_t lendata, void *bufhdr, size_t lenhdr, struct sockaddr_in *dst_addr);
PRIVATE ssize_t flushqueue (int ifudp, struct msg_queue *queue);
PRIVATE int wait_event(int timer_event, int itc_event, u_int8_t *event);

int wait_event(int timer_event, int itc_event, u_int8_t *event)
{
    fd_set read_event;
    int max, ret;    

    FD_ZERO(&read_event);
    FD_SET(output_timer,&read_event);
    FD_SET(itc_event, &read_event);
    
    max = (timer_event > itc_event) ? timer_event : itc_event;
    
    while(1) {
	ret = select(max+1, &read_event, NULL, NULL, NULL);
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
    if (FD_ISSET(timer_event, &read_event))
	*event |= TIMER_EXPIRATION;
    if (FD_ISSET(itc_event, &read_event))
	*event |= ITC_EVENT;
    
    return ret;
}

void *output (void *arg)
{
    struct msg_queue outputq[3];
    struct msg_queue txq;
    struct itc_event_info ieinfo;
    ssize_t   ret, tmp;
    size_t    n;
    char      *where;
    u_int8_t  event;		/* ITC Event	    */
    u_int64_t exp;		/* Expiration Times */
    int msg_nor_pending;	/* Flag if Event arrives */
    int msg_ret_pending;	/* Flag if Event arrives */  
    
    /*
     * All itc signal used for comunicacion MUST be blocked
     */
    itc_block_signal();

    init_msg_queue(&outputq[PRIO_CTR_QUEUE]);
    init_msg_queue(&outputq[PRIO_RET_QUEUE]);
    init_msg_queue(&outputq[PRIO_NOR_QUEUE]);
    init_msg_queue(&txq);

    bytes_sent = 0;
    msg_sent   = 0;

    msg_nor_pending = 0;
    msg_ret_pending = 0;
    while(1) 
    {
	ret = wait_event(output_timer, itc_event, &event);

	if (ret == -1) 
	{
	    where = "wait_event()";
	    goto panic;
	    
	}

	if (event & ITC_EVENT) 
	{
	    ret = itc_read_event(itc_event, &ieinfo);
	    if (ret == -1)
	    {
		where = "wait_event()";
		goto panic;
	    }
	    if (ieinfo.src != KERNEL_LAYER_THREAD)
	    {
		/* Debug */
		
	    }
	    if (ieinfo.prio == PRIO_CTR_QUEUE)
	    {
		itc_readfrom(KERNEL_LAYER_THREAD,
			     &outputq[PRIO_CTR_QUEUE],
			     PRIO_CTR_QUEUE);
		msgqcat(&txq, &outputq[PRIO_CTR_QUEUE]);
	    }

	    if (ieinfo.prio == PRIO_NOR_QUEUE)
	    {
		if (outputq[PRIO_NOR_QUEUE].size > 0)
		    msg_nor_pending = 1;
		else
		    itc_readfrom(KERNEL_LAYER_THREAD, 
			 &outputq[PRIO_NOR_QUEUE], 
			 PRIO_NOR_QUEUE );
	    }

	    if (ieinfo.prio == PRIO_RET_QUEUE)
	    {
		if (outputq[PRIO_RET_QUEUE].size > 0)
		    msg_ret_pending = 1;
		else
		    itc_readfrom(KERNEL_LAYER_THREAD, 
			 &outputq[PRIO_RET_QUEUE], 
			 PRIO_RET_QUEUE );
	    }

	}
	if (event & TIMER_EXPIRATION) 
	{
	    ret = gettimerexp(output_timer, &exp);
	    if (ret == -1) 
	    {
		where = "gettimerexp()";
		goto panic;
	    }
	    n = (size_t) exp;
	    /*
             * El timer expiro n veces
	     *  - Se deben enviar tantos mensajes como expiraciones
             */
	    if (outputq[PRIO_RET_QUEUE].size < n && msg_ret_pending) 
	    {
		itc_readfrom(KERNEL_LAYER_THREAD, 
			 &outputq[PRIO_RET_QUEUE], 
			 PRIO_RET_QUEUE );
		msg_ret_pending = 0;
	    }
	    n -= msgnmove(&txq, &outputq[PRIO_RET_QUEUE], n);

	    if (outputq[PRIO_NOR_QUEUE].size < n && msg_nor_pending) 
	    {
		itc_readfrom(KERNEL_LAYER_THREAD, 
			 &outputq[PRIO_NOR_QUEUE], 
			 PRIO_NOR_QUEUE );
		msg_nor_pending = 0;
	    }
	    msgnmove(&txq, &outputq[PRIO_NOR_QUEUE], n);
	}
	tmp = txq.size;
	ret = flushqueue(ifudp, &txq);
	if (ret == -1) 
	{
	    where = "flushqueue()";
	    goto panic;
	}    
	msg_sent   +=tmp;
	bytes_sent +=ret;

	if (msg_sent >= 8333)
	    pthread_exit(&ret);
    }

panic:
    PANIC(errno, "NETOUT_LAYER_THREAD", where);
    return NULL;
}

ssize_t flushqueue (int ifudp, struct msg_queue *queue)
{
    struct mmsghdr   mmsg[MAX_OUTPUT_MSG];
    struct iovec     io[MAX_OUTPUT_MSG*2];
    struct msg_queue tmpq;
    struct mbuff     *mbptr;
    struct msg	     *msgptr;
    ssize_t output_len;
    ssize_t ret;
    int i, qsz;

    output_len = 0;

    while (queue->size > 0)
    {
	if (queue->size == 1) 
	{
	    msgptr = queue->head;
	    mbptr =  queue->head->p_mbuff;

	    if (mbptr->m_need_ts == DO_TS)
		/*
		 * Put the timestamp 
		 */
		timestamp((u_int64_t *)&(mbptr->m_payload[mbptr->m_tsoff]), NULL);
	
	    ret = sendmbuff(ifudp, 
			    mbptr->m_payload, 
			    mbptr->m_datalen, 
			    &(mbptr->m_hdr),
			    mbptr->m_hdrlen,
			    &(mbptr->m_outside_addr));
	    if (ret == -1) 
	    {
		output_len = ret;
		break;
	    }
	    if (ret == mbptr->m_datalen + mbptr->m_hdrlen) {
		/*
		 * Esta comprobacion es absurda porque en UDP
		 * se deberia enviar todo lo que se le pidio en un datagrama
		 */
		output_len += ret;

		msgptr = msg_dequeue(queue);
		if (msgptr->discard == DISCARD_TRUE)
		    free_mbuff_locking(msgptr->p_mbuff);
		free_msg_locking(msgptr);
	    }
	}
	else 
	{
	    /*
	     * La funcion envia hasta 10 datagramas por envio
	     * Si supera los 10 se envian en la siguiente ronda
	     */
	    qsz = (queue->size > MAX_OUTPUT_MSG)? MAX_OUTPUT_MSG: queue->size;

	    tmpq.head = NULL;
	    tmpq.tail = NULL;
	    tmpq.size = 0;
	
	    for ( i = 0; i <= qsz -1 && i <= MAX_OUTPUT_MSG -1; i++ ) {
	    	
		msgptr = msg_dequeue(queue);
		mbptr  = msgptr->p_mbuff;
		if (mbptr->m_need_ts == DO_TS)
		    /*
		     * Put Timestamp
		     */
		    timestamp((u_int64_t *)&(mbptr->m_payload[mbptr->m_tsoff]), NULL);

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
	    {
		if (errno == EINTR)
		{
		    msgqcat(queue, &tmpq);
		    continue;
		}
		else {
		    output_len = ret;
		    break;
		}
	    }
	}
    }
    return output_len;
}



#define SENDTO_MAX_BUFFER 2048
ssize_t sendmbuff (int sd, void *bufdata, size_t lendata, void *bufhdr, size_t lenhdr,
			    struct sockaddr_in *dst_addr)
{
    ssize_t ret = 0;
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
    

    while (1) 
    {
	ret = sendmsg(sd,
		      &msg,
		      0);
	if ( ret > 0 || ( ret == -1 && errno != EINTR ))
	    break;
    }
#endif
    return ret;
}
