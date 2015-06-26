#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#define SENDTO_MAX_BUFFER 2048

ssize_t bbtp_finish_output (int sd, void *bufdata, size_t lendata, void *bufhdr, size_t lenhdr,
			    struct sockaddr_in *dst_addr, struct timespec *op_delay)
{
    ssize_t ret;
    struct timespec in, out;
    socklen_t alen = sizeof(struct sockaddr_in);
#ifdef USE_SENDTO
    unsigned char buf[SENDTO_MAX_BUFFER];
#else
    struct msghdr msg;
    struct iovec  io[2];
#endif

    if (op_delay != NULL)
	clock_gettime(CLOCK_MONOTONIC, &in);

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
		 MSG_DONTWAIT,
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
		  MSG_DONTWAIT);
#endif
    if (op_delay != NULL)
	clock_gettime(CLOCK_MONOTONIC, &out);

    if (op_delay != NULL)
    {
	op_delay->tv_sec  = out.tv_sec  - in.tv_sec;
	op_delay->tv_nsec = out.tv_nsec - in.tv_nsec;
    }
    return ret;
}
