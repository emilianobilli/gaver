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

#include <stdlib.h>
#include <stdio.h>
#include "mbuff.h"
#include "glo.h"
#include "heap.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/*======================================================================================*
 * dump_heap()										*
 *======================================================================================*/
void dump_heap(FILE *file)
{
    fprintf(file, "HEAP( mbuff: %ld, msg: %ld )\n", heap_mbuff_size(),heap_msg_size());
}

/*======================================================================================*
 * dump_output_mb()									*
 *======================================================================================*/
void dump_output_mb (FILE *file, struct mbuff *mb)
{
    struct in_addr in;
    char   ip_str_in[16];
    char   ip_str_out[16];

    in.s_addr = local_addr;
    fprintf(file,"%s from %s:%d:%d to %s:%d:%d (seq: %llu, type: %#x, len: %d)\n",
    	    "output",
	     inet_ntop(AF_INET, &in,ip_str_in, sizeof(struct sockaddr_in)),
	     ntohs(local_port),
	     ntohs(mb->m_hdr.src_port),
    	     inet_ntop(AF_INET, &(mb->m_outside_addr.sin_addr),ip_str_out, sizeof(struct sockaddr_in)),
	     ntohs(mb->m_outside_addr.sin_port),
	     ntohs(mb->m_hdr.dst_port),
	     (long long unsigned int)mb->m_hdr.seq,
	     mb->m_hdr.type,
	     mb->m_hdr.payload_len);

    return;
}
/*======================================================================================*
 * dump_input_mb()									*
 *======================================================================================*/
void dump_input_mb (FILE *file, struct mbuff *mb)
{
    struct in_addr in;
    char   ip_str_in[16];
    char   ip_str_out[16];
    in.s_addr = local_addr;
    fprintf(file,"%s from %s:%d:%d to %s:%d:%d (seq: %llu, type: %#x, len: %d)\n",
    	    "input",
	     inet_ntop(AF_INET, &(mb->m_outside_addr.sin_addr),ip_str_in, sizeof(struct sockaddr_in)),
	     ntohs(mb->m_outside_addr.sin_port),
	     ntohs(mb->m_hdr.src_port),
    	     inet_ntop(AF_INET, &in, ip_str_out, sizeof(struct sockaddr_in)),
	     ntohs(local_port),
	     ntohs(mb->m_hdr.dst_port),
	     (long long unsigned int) mb->m_hdr.seq,
	     mb->m_hdr.type,
	     mb->m_hdr.payload_len);
	
}
