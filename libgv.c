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
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include "gv_err.h"
#include "apitypes.h"


int gv_listen_api_msg (int sd, u_int8_t backlog)
{
    struct gv_msg_api msg;
    struct gv_ret_api ret;

    memset(&msg, 0, sizeof(struct gv_msg_api);
    msg.msg_type = MSG_CONNECT;
    msg.un.listen.backlog = backlog;

    if (rw_msg(sd, &msg, IOMSG_ACTION_WRITE) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    memset(&ret, 0, sizeof(struct gv_ret_api);
    if (rw_msg(sd, &ret, IOMSG_ACTION_READ) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    if (ret.status == COMMAND_FAIL)
    {
    	/*
         * Asignar GV ERROR
         */
	return -1;
    }
    return 0;
}


int gv_accept_api_msg(int sd, u_int32_t *addr, u_int16_t *port, u_int16_t *vport, char *sun_path)
{
    struct gv_msg_api msg;
    struct gv_ret_api ret;

    memset(&msg, 0, sizeof(struct gv_msg_api);

    msg.msg_type = MSG_ACCEPT;
    strcpy(msg.un.accept.sun_path, sun_path);

    if (rw_msg(sd, &msg, IOMSG_ACTION_WRITE) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    memset(&ret, 0, sizeof(struct gv_ret_api);
    if (rw_msg(sd, &ret, IOMSG_ACTION_READ) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    if (ret.status == COMMAND_FAIL)
    {
    	/*
         * Asignar GV ERROR
         */
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
    struct gv_msg_api msg;
    struct gv_ret_api ret;

    memset(&msg, 0, sizeof(struct gv_msg_api);

    msg.msg_type = MSG_BIND;
    msg.un.bind.addr  = *addr;
    msg.un.bind.port  = *port;
    msg.un.bind.vport = *vport;

    if (rw_msg(sd, &msg, IOMSG_ACTION_WRITE) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    memset(&ret, 0, sizeof(struct gv_ret_api);
    if (rw_msg(sd, &ret, IOMSG_ACTION_READ) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    if (ret.status == COMMAND_FAIL)
    {
    	/*
         * Asignar GV ERROR
         */
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
    struct gv_msg_api msg;
    struct gv_ret_api ret;

    memset(&msg, 0, sizeof(struct gv_msg_api);

    msg.msg_type = MSG_CONNECT;
    msg.un.connect.addr  = addr;
    msg.un.connect.port  = port;
    msg.un.connect.vport = vport;
    strcpy(msg.un.sun_path, sun_path);

    if (rw_msg(sd, &msg, IOMSG_ACTION_WRITE) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    memset(&ret, 0, sizeof(struct gv_ret_api);
    if (rw_msg(sd, &ret, IOMSG_ACTION_READ) == -1)
	/*
         * Asignar GV ERROR
         */
	return -1;

    if (ret.status == COMMAND_FAIL)
    {
    	/*
         * Asignar GV ERROR
         */
	return -1;
    }
    return 0;
}



ssize_t rw_msg(int sd, struct gv_msg_api *msg, int action);


ssize_t rw_msg(int sd, struct gv_msg_api *msg, int action)
{
    ssize_t bytes_transfd, ret;
    u_int8_t *ptr;
    
    ptr = (u_int8_t *) &msg;
    bytes_transfd = 0;

    if (action == IOMSG_ACTION_READ )
    {
	while(bytes_transfd < GVMSGAPISZ)
	{
	    ret = recv(sd, &ptr[bytes_transfd], GVMSGAPISZ - bytes_transfd, 0);
	    if (ret == -1) {
		if (errno != EINTR) 
		    return -1;
		else
		    continue;
	    }
	    bytes_transfd += ret;
	}
    }
    if (action == IOMSG_ACTION_WRITE)
    {
	while(bytes_transfd < GVMSGAPISZ)
	{
	    ret = send(sd, &ptr[bytes_transfd], GVMSGAPISZ - bytes_transfd, 0);
	    if (ret == -1) {
		if (errno != EINTR) 
		    return -1;
		else
		    continue;
	    }
	    bytes_transfd += ret;
	}
    }
    return bytes_transfd;
}
