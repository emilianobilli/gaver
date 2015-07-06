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
#ifndef _MBUFF_H
#define _MBUFF_H

#include <sys/types.h>
#include <netinet/in.h>
#include "gaver.h"


#define DO_TS   1		/* When is necesary put TS */
#define DONT_TS 0

struct mbuff {
    struct mbuff       *m_next;
    struct sockaddr_in 	m_outside_addr;
    struct gvhdr	m_hdr;
    int 		m_need_ts;		/* Need Timestamp in outgoing  */
    off_t		m_tsoff;		/* Timestamp offset */
    u_int64_t		m_input_ts[2];		/* Input TS         */
    size_t		m_hdrlen;
    size_t		m_datalen;
    unsigned char	m_payload[PAYLOAD_SIZE];
};


struct mb_queue {
    size_t	 size;
    struct mbuff *head;
    struct mbuff *tail;
};

#define DISCARD_TRUE  1
#define DISCARD_FALSE 0

#define MSG_TYPE_REQ     0		/* Request */
#define MSG_TYPE_REP     1		/* Reply   */
#define MSG_TYPE_CARRIER 2		/* Portadora */

#define OPT_READ	0
#define OPT_WRITE	1
#define OPT_CLOSE	2

struct opt_request {
    int    to;
    int    opt_type;		/* Read or Write or Close */
    size_t mtu;			/* If the type is Read */
};

struct opt_reply {
    int     from;
    int	    opt_type;		/* Type of request */
    ssize_t result;		/* size || -errno */
};

struct msg {
    int    msg_type;		/* Request or Reply */
    union {
	struct opt_request request;
	struct opt_reply   reply;
    } opt;
    int    discard;		/* If the mbuff or queue needs to be discard */
    
    union {
	struct mbuff    *p_mbuff;
	struct mb_queue  mbq;
    } mb;			/* Queue of mbuff or a simple mbuff */
    struct msg          *p_next;
};

struct msg_queue {
    size_t     size;
    struct msg *head;
    struct msg *tail;
};


#endif