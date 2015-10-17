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
#ifndef _LIBGV_H
#define _LIBGV_H

#include <sys/types.h>

extern int gv_listen_api_msg  (int sd, u_int8_t backlog);
extern int gv_accept_api_msg  (int sd, u_int32_t *addr, u_int16_t *port, u_int16_t *vport, char *sun_path);
extern int gv_bind_api_msg    (int sd, u_int32_t *addr, u_int16_t *port, u_int16_t *vport);
extern int gv_connect_api_msg (int sd, u_int32_t addr, u_int16_t port, u_int16_t vport, char *sun_path);

#endif