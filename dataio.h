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
#ifndef _DATAIO_H
#define _DATAIO_H

#define ATOMIC_READ 1024*1024		/* 1 MB */


#endif



ssize_t recvdata (int sdux, struct mb_queue *q, size_t mtu )
{
    struct msghdr msg;
    struct iovec *iodata;
    struct mb_queue t1, t2;
    struct mbuff *mbptr;
    ssize_t ret;
    size_t btor, bytes_recv;
    int value;


    t1.size = 0;
    t1.head = NULL;
    t1.tail = NULL;
    t2.size = 0;
    t2.head = NULL;
    t2.tail = NULL;
    

    /* Averiguar cuantos bytes hay en cola */
    if (ioctl(sdux, SIOCINQ, &value) == -1)
	return (ssize_t)-1;

    btor = ( value > ATOMIC_READ ) ? ATOMIC_READ : value;

    if (alloc_mbuff_payload(&t1, btor, mtu) == 0)
	return (ssize_t)0;

    iodata = (struct iovec *)calloc(sizeof(struct iovec), t1.size);
    if (iodata == NULL)
	return (ssize_t)0;

    
    i = 0;
    while (tmp.size != 0)
    {
	mbptr = mbuff_dequeue(&t1);
	iodata[i].iov_base = mbptr->m_payload;
	iodata[i].iov_len  = mtu;
	mbuff_enqueue(&t2);
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
	mbptr = mbuff_dequeue(&t2);
	free_mbuff_locking(mbptr);
    }
    return bytes_recv;
}


ssize_t senddata (int sdux, struct mb_queue *txq, struct mb_queue *no_sent)
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