
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


struct mbuff {
    struct mbuff     	*m_next;
    struct sockaddr_in 	m_outside_addr;
    struct gvhdr	m_hdr;
    size_t		m_hdrlen;
    size_t		m_datalen;
    unsigned char	m_payload[PAYLOAD_SIZE];
};

struct msg {
    struct msg    *p_next;
    struct mbuff  *p_mbuff;
};

struct msg_queue {
    size_t     size;
    struct msg *head;
    struct msg *tail;
};

struct mb_queue {
    size_t	 size;
    struct mbuff *head;
    struct mbuff *tail;
};

#endif