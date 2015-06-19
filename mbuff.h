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

#include <netinet/in.h>
#include "gaver.h"

typedef u_int8_t addr_type;

struct sounaddr {
    int socket;
};

struct mbuff
{
    struct m_buff     *m_next;
    struct m_buff     *m_prev;
#define ADDR_TYPE_INSISE  0x01
#define ADDR_TYPE_OUTSIDE 0x02
#define ADDR_TYPE_EMPTY   0x03
    addr_type	       m_atype;
    union {
	struct sounaddr    inside_addr;	
	struct sockaddr_in outsize_addr;
    } m_addr;
    struct gaverhdr    m_hdr;
    size_t	       m_datalen;
    unsigned char      m_payload[PAYLOAD_SIZE];
};

struct pktbuff
{
    struct pktbuff    *p_next;
    struct mbuff      *p_mbuff;
};


struct pktqueue
{
    struct pkbuff     *pq_head;
    struct pkbuff     *pq_tail;
};


#endif