/*
    This file is part of GaVer

    GaVer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GaVer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GaVer.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "gv_err.h"
#include "common.h"
#include "apitypes.h"

int gv_listen_api_msg (int sd, u_int8_t backlog)
{
    struct gv_req_api msg;
    struct gv_rep_api ret;

    memset(&msg, 0, sizeof(struct gv_req_api));

    msg.msg_type = MSG_CONNECT;
    msg.un.listen.backlog = backlog;

    if (write_msg(sd, &msg, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }
    memset(&ret, 0, sizeof(struct gv_rep_api));
    if (read_msg(sd, &ret, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }

    if (ret.status == COMMAND_FAIL)
    {
    	gv_errno = (int) ret.un.fail.error_code;
	return -1;
    }
    return 0;
}


int gv_accept_api_msg(int sd, u_int32_t *addr, u_int16_t *port, u_int16_t *vport, char *sun_path)
{
    struct gv_req_api msg;
    struct gv_rep_api ret;

    memset(&msg, 0, sizeof(struct gv_req_api));

    msg.msg_type = MSG_ACCEPT;
    strcpy((char *)msg.un.accept.sun_path, sun_path);

    if (write_msg(sd, &msg, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }

    memset(&ret, 0, sizeof(struct gv_rep_api));
    if (read_msg(sd, &ret, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }
    if (ret.status == COMMAND_FAIL)
    {
	gv_errno = (int) ret.un.fail.error_code;
	return -1;
    }
    else
    {
    	*addr  = ret.un.success.addr;
	*port  = ret.un.success.port;
	*vport = ret.un.success.vport;
    }
    return 0;
}


int gv_bind_api_msg (int sd, u_int32_t *addr, u_int16_t *port, u_int16_t *vport)
{	
    struct gv_req_api msg;
    struct gv_rep_api ret;

    memset(&msg, 0, sizeof(struct gv_req_api));

    msg.msg_type = MSG_BIND;
    msg.un.bind.addr  = *addr;
    msg.un.bind.port  = *port;
    msg.un.bind.vport = *vport;

    if (write_msg(sd, &msg, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }
    memset(&ret, 0, sizeof(struct gv_rep_api));
    if (read_msg(sd, &ret, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }
    if (ret.status == COMMAND_FAIL)
    {
    	gv_errno = (int) ret.un.fail.error_code;
	return -1;
    }
    else
    {
    	*addr  = ret.un.success.addr;
	*port  = ret.un.success.port;
	*vport = ret.un.success.vport;
    }
    return 0;
}


int gv_connect_api_msg (int sd, u_int32_t addr, u_int16_t port, u_int16_t vport, char *sun_path)
{
    struct gv_req_api msg;
    struct gv_rep_api ret;

    memset(&msg, 0, sizeof(struct gv_req_api));

    msg.msg_type = MSG_CONNECT;
    msg.un.connect.addr  = addr;
    msg.un.connect.port  = port;
    msg.un.connect.vport = vport;
    strcpy((char *)msg.un.connect.sun_path, sun_path);

    if (write_msg(sd, &msg,GVMSGAPISZ ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }
    memset(&ret, 0, sizeof(struct gv_rep_api));
    if (read_msg(sd, &ret, GVMSGAPISZ) == -1)
    {
	gv_errno = OSERROR;
	return -1;
    }
    if (ret.status == COMMAND_FAIL)
    {
	gv_errno = (int) ret.un.fail.error_code;
	return -1;
    }
    return 0;
}
