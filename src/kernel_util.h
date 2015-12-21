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

#ifndef _KERNEL_UTIL_H
#define _KERNEL_UTIL_H

#include "sock.h"
#include "mbuff.h"

#ifdef _KERNEL_UTIL_CODE
#undef EXTERN
#define EXTERN 
#endif

/*======================================================================================*
 * new_sk()										*
 *======================================================================================*/
EXTERN struct sock *new_sk( int sd );

/*======================================================================================*
 * kevent_init()									*
 *======================================================================================*/
EXTERN void do_update_tokens (struct sockqueue *sk, u_int64_t times);

/*======================================================================================*
 * do_collect_mbuff_to_transmit()							*
 *======================================================================================*/
EXTERN void do_collect_mbuff_to_transmit (struct sockqueue *sk, struct msg_queue *tx, struct msg_queue *txctrl);

/*======================================================================================*
 * do_accept_connection()								*
 *======================================================================================*/
EXTERN struct msg *do_accept_connection (struct sock *sk, struct mbuff *conn_req, int ctrl_ack);

/*======================================================================================*
 * do_process_sent_msg()								*
 *======================================================================================*/
EXTERN void do_process_sent_msg (struct msg_queue *sentq);

/*======================================================================================*
 * do_process_expired()									*
 *======================================================================================*/
EXTERN void do_process_expired (struct msg_queue *ctrq);

/*======================================================================================*
 * do_process_input_bulk()								*
 *======================================================================================*/
EXTERN void do_process_input_bulk (struct msg_queue *inputq, struct msg_queue *ctrq);

/*======================================================================================*
 * get_destination_port()								*
 *======================================================================================*/
EXTERN u_int16_t get_destination_port(struct mbuff *m);

/*======================================================================================*
 * get_destination_port_from_msg()							*
 *======================================================================================*/
EXTERN u_int16_t get_source_port_from_msg(struct msg *m);

/*======================================================================================*
 * get_source_port()									*
 *======================================================================================*/
EXTERN u_int16_t get_source_port(struct mbuff *m);

/*======================================================================================*
 * get_type_from_msg()									*
 *======================================================================================*/
EXTERN u_int8_t get_type_from_msg(struct msg *m);

/*======================================================================================*
 * get_type()										*
 *======================================================================================*/
EXTERN u_int8_t get_type(struct mbuff *m);

/*======================================================================================*
 * get_seq()										*
 *======================================================================================*/
EXTERN u_int64_t get_seq(struct mbuff *m);

#endif