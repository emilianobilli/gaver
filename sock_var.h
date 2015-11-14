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
#ifndef _SOCK_VAR_H
#define _SOCK_VAR_H

#include "sock.h"

EXTERN struct sock sktable[MAX_SOCKETS];
EXTERN struct sock *sk_gvport[sizeof(u_int16_t)];
EXTERN struct sockqueue so_used;
EXTERN struct sockqueue so_free;


#endif /* sock_var.h */
