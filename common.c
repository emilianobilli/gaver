/*
    Copyright (C) 2015 Emiliano A. Billi 
    
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
#define _COMMON_CODE
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "common.h"

ssize_t rw_msg(int sd, void *msg, u_int8_t action, size_t size)
{
    ssize_t bytes_transfd, ret;
    u_int8_t *ptr;
    
    ptr = (u_int8_t *) msg;
    bytes_transfd = 0;

    if (action == IOMSG_ACTION_READ )
    {
	while(bytes_transfd < size)
	{
	    ret = recv(sd, &ptr[bytes_transfd], size - bytes_transfd, MSG_WAITALL);
	    if (ret == -1) {
		if (errno != EINTR) 
		    return -1;
		else
		    continue;
	    }
	    else
		if (ret == 0)
		    return 0;
	    bytes_transfd += ret;
	}
    }
    if (action == IOMSG_ACTION_WRITE)
    {
	while(bytes_transfd < size)
	{
	    ret = send(sd, &ptr[bytes_transfd], size - bytes_transfd, MSG_NOSIGNAL);
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

ssize_t write_msg(int sd, void *msg, size_t size)
{
    return rw_msg(sd,msg, IOMSG_ACTION_WRITE, size);
}

ssize_t read_msg(int sd, void *msg, size_t size)
{
    return rw_msg(sd,msg, IOMSG_ACTION_READ, size);
}


