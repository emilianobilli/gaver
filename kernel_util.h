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
 * prepare_syn()									*
 *======================================================================================*/
EXTERN struct msg *prepare_syn (struct sock *sk);


/*======================================================================================*
 * prepare_connect()									*
 *======================================================================================*/
EXTERN struct msg *prepare_connect (struct sock *sk);


/*======================================================================================*
 * prepare_accept()									*
 *======================================================================================*/
EXTERN struct msg *prepare_accept (struct sock *sk);

/*======================================================================================*
 * do_process_sent_msg()								*
 *======================================================================================*/
EXTERN void do_process_sent_msg (struct msg_queue *sentq);

#endif