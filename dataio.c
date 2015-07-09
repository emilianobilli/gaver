/*
    This file is part of GaVer

    GaVer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GaVer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#define _DATAIO_CODE
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "dataio.h"
#include "itc.h"
#include "types.h"
#include "glo.h"
#include "mbuff.h"
#include "mbuff_queue.h"
#include "heap.h"
#include "util.h"
#include "gaver.h"


PRIVATE ssize_t recvdata (int sdux, struct mb_queue *q, size_t len, size_t mtu );
PRIVATE ssize_t senddata (int sdux, struct mb_queue *txq, struct mb_queue *no_sent, int discard);
PRIVATE int select_nosignal (int max, fd_set *read, fd_set *write, fd_set *excep, struct timeval *tout);
PRIVATE int fill_fdset (struct msg_queue *queue, fd_set *set);

int select_nosignal ( int max, fd_set *read, fd_set *write, fd_set *except, struct timeval *tout)
{
    int ret;
    while (1)
    {
	    ret = select(max+1, read, write, except, tout);
	    if ( ret >= 0 || (ret == -1 && errno != EINTR ))
		break;	
    }
    return ret;
}

int fill_fdset (struct msg_queue *queue, fd_set *set)
{
    struct msg_queue tmp;
    struct msg *mptr;
    int    max = 0;

    FD_ZERO(set);

    init_msg_queue(&tmp);

    while( (mptr = msg_dequeue(queue))!= NULL )
    {
	FD_SET(mptr->io.io_socket, set);
	max = ( max > mptr->io.io_socket )? max : mptr->io.io_socket;
	msg_enqueue(&tmp, mptr);
    }
    msgqcat(queue, &tmp);

    return max;
}


void *dataio (void *arg)
{
    struct msg_queue wpenq;		/* Pendientes de escritura */
    struct msg_queue rpenq;		/* Pendientes de lectura   */
    struct msg_queue to_krn;
    struct msg_queue tmp;
    struct mb_queue  no_sent;
    struct itc_event_info ieinfo;
    struct msg	     *mptr;
    ssize_t io_ret;
    int max, ret;
    fd_set wset, rset;
    char   *where;

    itc_block_signal();

    init_msg_queue(&wpenq);
    init_msg_queue(&rpenq);

    while (1)
    {
	FD_ZERO(&wset);
	FD_ZERO(&rset);
	/*
         * Add itc file descriptor
         */
	FD_SET(itc_event, &rset);
	max = itc_event;

	/*
	 * Add all sockets scaning read events 
	 */
	
	ret = fill_fdset(&rpenq, &rset);
	max = ( max > ret ) ? max : ret;
	/*
         * Add all sockets scaning write envents
         */
	ret = fill_fdset(&wpenq, &wset);
	max = ( max > ret ) ? max : ret;
	
	ret = select_nosignal(max+1, &rset, &wset, NULL, NULL);
	if (ret == -1)
	{
	    where = "select_nosignal()";
	    goto panic;
	}
	init_msg_queue(&tmp);
	init_msg_queue(&to_krn);

	while ((mptr = msg_dequeue(&wpenq)) != NULL)
	{
	    if (FD_ISSET(mptr->io.io_socket, &wset)) {
		io_ret = senddata(mptr->io.io_socket,
				 &mptr->mb.mbq,
				 &no_sent, DISCARD_TRUE);
		
		if (io_ret > 0 && no_sent.size == 0) {
		    mptr->io.io_opt = IO_OPT_WRITE;
		    mptr->io.io_ret = IO_RET_SUCCESS;
		    mptr->io.io_errno   = 0;
		    mptr->io.io_rep_len += io_ret;

		    mptr->msg_type = MSG_IO_REPLY;

		    msg_enqueue(&to_krn, mptr);
		}
		else if ( io_ret == -1 ) {
		    mptr->io.io_opt = IO_OPT_WRITE;
		    mptr->io.io_ret = IO_RET_FAILURE;
		    mptr->io.io_errno = errno;
		
		    mptr->msg_type = MSG_IO_REPLY;
		    mbuffqcat(&(mptr->mb.mbq), &no_sent);
		    msg_enqueue(&to_krn, mptr);
		}
		else {
		    mptr->io.io_rep_len += io_ret;
		    mbuffqcat(&(mptr->mb.mbq), &no_sent);
		    msg_enqueue(&tmp, mptr);
		}
	    }
	    else
		msg_enqueue(&tmp, mptr);
	}
	msgqcat(&wpenq, &tmp);

	init_msg_queue(&tmp);

	while ((mptr = msg_dequeue(&rpenq)) != NULL)
	{
	    if (FD_ISSET(mptr->io.io_socket, &rset)) {
		io_ret = recvdata(mptr->io.io_socket,
				 &(mptr->mb.mbq),
				 mptr->io.io_req_len, mptr->io.io_chunk_size);
		
		if (io_ret > 0) {
		    mptr->io.io_opt = IO_OPT_READ;
		    mptr->io.io_ret = IO_RET_SUCCESS;
		    mptr->io.io_errno   = 0;
		    mptr->io.io_rep_len = io_ret;

		    mptr->msg_type = MSG_IO_REPLY;

		    msg_enqueue(&to_krn, mptr);
		}
		else if ( io_ret == -1 ) {
		    mptr->io.io_opt = IO_OPT_READ;
		    mptr->io.io_ret = IO_RET_FAILURE;
		    mptr->io.io_errno = errno;
		    mptr->msg_type = MSG_IO_REPLY;
		    msg_enqueue(&to_krn, mptr);
		}
		else {
		    msg_enqueue(&tmp, mptr);
		}
	    }
	    else
		msg_enqueue(&tmp, mptr);
	}
	msgqcat(&rpenq, &tmp);

	init_msg_queue(&tmp);

	/*
	 * Look for new request
	 */
	if (FD_ISSET(itc_event, &rset)) 
	{
	    ret = itc_read_event(itc_event, &ieinfo);
	    if (ret == -1)
	    {
		where = "itc_read_event()";
		goto panic;
	    }

	    if (ieinfo.src != KERNEL_LAYER_THREAD)
	    {
		where = "[itc wrong src]";
		goto panic;
	    }
	    
	    itc_readfrom(KERNEL_LAYER_THREAD,
		         &tmp,
			 0);

	    while( (mptr = msg_dequeue(&tmp)) != NULL)
	    {
		if (mptr->msg_type != MSG_IO_REQUEST)
		    /* Drop silently */
		    continue;
		mptr->io.io_ret     = 0;
		mptr->io.io_errno   = 0;
		mptr->io.io_rep_len = 0;
		if (mptr->io.io_opt == IO_OPT_READ)
		    msg_enqueue(&rpenq, mptr);
		if (mptr->io.io_opt == IO_OPT_WRITE)
		    msg_enqueue(&wpenq, mptr);
	    }
	}

	/*
         * Send the reply to the kernel
	 */
	if (to_krn.size != 0)
	    itc_writeto(KERNEL_LAYER_THREAD,
			&to_krn,
			0);
    }

panic:
    PANIC(errno,"DATAIO_LAYER_THREAD",where);
    return NULL;
}


ssize_t recvdata (int sdux, struct mb_queue *q, size_t len, size_t mtu )
{
    struct msghdr msg;
    struct iovec *iodata;
    struct mb_queue t1, t2;
    struct mbuff *mbptr;
    ssize_t ret;
    size_t btor, bytes_recv;
    int value, i;
    

    t1.size = 0;
    t1.head = NULL;
    t1.tail = NULL;
    t2.size = 0;
    t2.head = NULL;
    t2.tail = NULL;
    

    /* Averiguar cuantos bytes hay en cola */
    if (ioctl(sdux, SIOCINQ, &value) == -1)
	return (ssize_t)-1;
    btor = ( ATOMIC_READ > len ) ? len : ATOMIC_READ;
    btor = ( value > btor ) ? btor : value;

    if (alloc_mbuff_payload(&t1, btor, mtu) == 0)
	return (ssize_t)0;

    iodata = (struct iovec *)calloc(sizeof(struct iovec), t1.size);
    if (iodata == NULL)
	return (ssize_t)0;
    
    i = 0;
    while (t1.size != 0)
    {
	mbptr = mbuff_dequeue(&t1);
	iodata[i].iov_base = mbptr->m_payload;
	iodata[i].iov_len  = mtu;
	mbuff_enqueue(&t2, mbptr);
	i++;
    }

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iodata;   /* scatter/gather array */
    msg.msg_iovlen = i;     /* # elements in msg_iov */
    msg.msg_control = NULL; /* ancillary data, see below */
    msg.msg_controllen = 0; /* ancillary data buffer len */
    msg.msg_flags = 0;      /* flags on received message */


    while (1)
    {
	ret = recvmsg(sdux,
   	              &msg,
		      MSG_NOSIGNAL | MSG_DONTWAIT);
	if ( ret > 0 || (ret == -1 && errno == EINTR))
	    break;
    }

    free(iodata);
    if (ret == -1) 
    {
	if (errno == EAGAIN || errno == EWOULDBLOCK) 
	    return (ssize_t)0;
	else
	    return (ssize_t)-1;
    }

    bytes_recv = ret;
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
    while(ret > 0)
    {
	mbptr = mbuff_dequeue(&t2);
	if (ret >= mtu) 
	{
	    mbptr->m_datalen = mtu;
	    ret -= mtu;
	}
	else
	{
	    mbptr->m_datalen = ret;
	    ret = 0;
	}
	mbuff_enqueue(q, mbptr);
    }
    while (t2.size != 0)
    {
	/*
         * Elimina lo que no utilizo
         */
	mbptr = mbuff_dequeue(&t2);
	free_mbuff_locking(mbptr);
    }
    return bytes_recv;
}


ssize_t senddata (int sdux, struct mb_queue *txq, struct mb_queue *no_sent, int discard)
{
    struct msghdr msg;
    struct iovec *iodata;
    struct mb_queue  tmp;
    struct mbuff *mbptr;
    ssize_t ret;
    size_t  bytes_sent;
    int i;

    tmp.size = 0;
    tmp.head = NULL;
    tmp.tail = NULL;

    no_sent->size = 0;
    no_sent->head = NULL;
    no_sent->tail = NULL;

    iodata = (struct iovec *) calloc(sizeof(struct iovec), txq->size);

    if (iodata == NULL)
    {
	mbuffqcat(no_sent, txq);
	return (ssize_t)0;
    }

    /*
     * Fill the struct
     */
    i = 0;
    while ( txq->size != 0 )
    {
	mbptr = mbuff_dequeue(txq);

	iodata[i].iov_base	= mbptr->m_payload;
	iodata[i].iov_len	= mbptr->m_datalen;

	mbuff_enqueue(&tmp, mbptr);
	i++;
    }
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iodata;   /* scatter/gather array */
    msg.msg_iovlen = i;     /* # elements in msg_iov */
    msg.msg_control = NULL; /* ancillary data, see below */
    msg.msg_controllen = 0; /* ancillary data buffer len */
    msg.msg_flags = 0;      /* flags on received message */

    while (1)
    {
	ret = sendmsg(sdux,
   	              &msg,
		      MSG_NOSIGNAL | MSG_DONTWAIT);
	if ( ret > 0 || (ret == -1 && errno == EINTR))
	    break;
    }

    free(iodata);
    if (ret == -1) 
    {
	mbuffqcat(no_sent, &tmp);
	if (errno == EAGAIN || errno == EWOULDBLOCK) 
	    return 0;
	else
	    return -1;
    }

    bytes_sent = ret;

    while ( ret > 0 )
    {
	mbptr = mbuff_dequeue(&tmp);
	if ( ret >= mbptr->m_datalen )
	{
	    ret -= mbptr->m_datalen;
	    if (discard)
		free_mbuff_locking(mbptr);
	}
	else
	{
	    memmove(&(mbptr->m_payload[0]), &(mbptr->m_payload[ret]), mbptr->m_datalen - ret);
	    mbptr->m_datalen -= ret;
	    ret = 0;
	    mbuff_enqueue(no_sent, mbptr);
	}
    }
    mbuffqcat(no_sent, &tmp);
    return bytes_sent;
}