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
#ifndef _API_TYPES_H
#define _API_TYPES_H

#include <sys/types.h>

#define SUN_PATH_SIZE		108
#define GVMSGAPISZ		128
#define COMMAND_SUCCESS		0
#define COMMAND_FAIL		1

#define MSG_CONNECT		0x00
#define MSG_ACCEPT		0x01
#define MSG_BIND		0x02
#define MSG_LISTEN		0x03
#define MSG_CLOSE		0x04
#define MSG_SETSPEED		0x05
#define MSG_GETSPEED		0x06


#define PADDING(x,y) u_int8_t _pad[GVMSGAPISZ-sizeof(x)-sizeof(y)]


struct gv_msg_connect {
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;	/* Port of Gaver */
    u_int8_t  sun_path[SUN_PATH_SIZE];
};


struct gv_msg_accept {
    u_int8_t  sun_path[SUN_PATH_SIZE];
};


struct gv_msg_bind {
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;
};

struct gv_msg_setspeed {
    u_int64_t speed;
};

struct gv_msg_listen {
    u_int8_t  backlog;
};


union gv_reply {
    struct {
	u_int16_t  error_code;		/* 2 bytes */
    } fail;
    struct {
	u_int32_t addr;			/* 4 bytes */
	u_int16_t port;			/* 2 bytes */
	u_int16_t vport;		/* 2 bytes */
	u_int64_t value;		/* 8 bytes for generic return */
    } success;
};

union gv_request {
    struct gv_msg_connect  connect;	
    struct gv_msg_accept   accept;
    struct gv_msg_bind     bind;
    struct gv_msg_listen   listen;
    struct gv_msg_setspeed speed;
};

struct gv_req_api {
    u_int8_t         msg_type;
    PADDING(union gv_request,u_int8_t);
    union gv_request un;
};


struct gv_rep_api {
    u_int8_t         status;
    PADDING(union gv_reply,u_int8_t);
    union gv_reply  un;
};

#endif /* apitypes.h */
