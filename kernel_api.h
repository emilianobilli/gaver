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

#ifndef _KERNEL_API_H
#define _KERNEL_API_H

#include "mbuff.h"
#include "sock.h"
#include "types.h"

EXTERN int do_socket_request(struct sock *sk, struct msg_queue *txq);
EXTERN int do_socket_error_response(struct sock *sk, int reason);
#endif