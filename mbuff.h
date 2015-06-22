
/*
 * ToDo: 
 *	- Modificar la estructra msg y agregar, direccion de origen (socket)
 *	- Agregar contador de elementos en cadena
 *	- Reemplazar puntero a mbuff por cola de mbuff
 *	- Modificar todo heap para estos ultimos cambios
 */

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

typedef u_int8_t addr_type;

struct sounaddr {
    int socket;
};

struct mbuff {
    struct mbuff      *m_next;
#define ADDR_TYPE_INSISE  0x01
#define ADDR_TYPE_OUTSIDE 0x02
#define ADDR_TYPE_EMPTY   0x03
    addr_type	       m_atype;
    union {
	struct sounaddr    inside_addr;	
	struct sockaddr_in outsize_addr;
    } m_addr;
    struct gvhdr       m_hdr;
    size_t	       m_datalen;
    unsigned char      m_payload[PAYLOAD_SIZE];
};

struct msg {
    struct msg    *p_next;
    struct mbuff  *p_mbuff;
    /* struct mb_queue *mbq; */
    
};

struct msg_queue {
    struct msg *head;
    struct msg *tail;
};

struct mb_queue {
    struct mbuff *head;
    struct mbuff *tail;
};

#endif