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
#include "defs.h"
#include "gaver.h"


#define DO_TS   1		/* When is necesary put TS */
#define DONT_TS 0

#define MBUFF_READY 1


struct mbuff {
    struct mbuff       *m_next;
    struct sockaddr_in 	m_outside_addr;
    struct gvhdr	m_hdr;
    int 		m_need_ts;		/* Need Timestamp in outgoing  */
    off_t		m_tsoff;		/* Timestamp offset */
    u_int64_t		m_input_ts[2];		/* Input TS         */
    size_t		m_hdrlen;
    size_t		m_datalen;
    unsigned char	m_payload[PAYLOAD_SIZE(MAX_MTU_SIZE)];
};


struct mb_queue {
    size_t	 size;
    struct mbuff *head;
    struct mbuff *tail;
};

#define DISCARD_TRUE  1
#define DISCARD_FALSE 0

#define MSG_IO_REQUEST    0		/* Request */
#define MSG_IO_REPLY      1		/* Reply   */
#define MSG_MBUFF_CARRIER 2		/* Portadora */

#define IO_OPT_READ	0
#define IO_OPT_WRITE	1

#define IO_RET_SUCCESS  0
#define IO_RET_FAILURE  1

#define SENT_SUCCESS    0
#define SENT_ERROR      1

struct msg {
    int    msg_type;			/* Request, Reply or Carrier */
    struct {
	int io_opt;
	int io_socket;
	int io_ret;
	int io_errno;
	size_t io_req_len;
	size_t io_rep_len;
	size_t io_chunk_size;
    } io;
    int    sent_result;			/* Sent result at output */
    int    sent_error;			/* Sent error response   */
    int    discard;			/* If the mbuff or queue needs to be discard */
    union {
	struct mbuff    *mbp;		/* Pointer to a simple mbuff */
	struct mb_queue  mbq;		/* Queue of mbuff            */
    } mb;				/* Queue of mbuff or a simple mbuff */
    struct msg          *p_next;
};

struct msg_queue {
    size_t     size;
    struct msg *head;
    struct msg *tail;
};


#endif