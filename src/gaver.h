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

#ifndef _GAVER_H
#define _GAVER_H

#include <stddef.h>
#include <sys/types.h>

#define GAVER_PROTOCOL_VERSION 1


/*
 * GaVer Header
 */
struct gvhdr {
    u_int16_t src_port;		/* Virtual Source Port */
    u_int16_t dst_port;		/* Virtual Destination Port */
    u_int16_t payload_len;
    u_int8_t  version;
    u_int8_t  type;
    u_int64_t seq;		/* Data/Ctrl seq */
    u_int64_t txts_sec;
    u_int64_t txts_nsec;
};


#define DATA		0x00	/* 0000 0000 */
#define SYN		0x01	/* 0000 0001 */
#define ACK		0x02	/* 0000 0010 */
#define NACK		0x04	/* 0000 0100 */
#define CONNECT		0x08	/* 0000 1000 */
#define FINISH		0x10	/* 0001 0000 */
#define ACCEPT		0x20    /* 0010 0000 */
#define CTRL_ACK	0x40    /* 0100 0000 */

#define SYN_ACK       (SYN | ACK)
#define SYN_ACK_NACK  (SYN | ACK | NACK)


struct gvctrlack {
    u_int64_t ctrl_seq_ack;
    u_int64_t peer_ts_sec;
    u_int64_t peer_ts_nsec;
    u_int64_t rx_ts_sec;
    u_int64_t rx_ts_nsec;
};


/*
 * Es la misma informacion para CONNECT que para ACCEPT
 */
struct gvconn {
    u_int64_t data_seq;
    u_int64_t speed;
    u_int32_t recv_window;
    u_int16_t mtu;
    u_int8_t  speed_type;
    u_int8_t  _res;
};



/*
 * GaVer Syn Payload
 */
struct gvsyn {
    u_int64_t current_speed;
    u_int32_t recv_window;
    u_int32_t reserved;
    u_int64_t seq_ack;
};


#define SPEED_FAIR	0x00
#define SPEED_FIX	0x01

/*
 * GaVer Finish Payload
 */

struct gvfinish {
    u_int64_t ctrl_end_seq;
    u_int64_t data_end_seq;
};

/*
 * Protocol state
 */
enum {
    GV_ESTABLISHED = 1,
    GV_CONNECT_RCVD,
    GV_CONNECT_SENT,
    GV_ACCEPT_SENT,
    GV_CLOSE,
    GV_LISTEN,
    GV_FINISH_SENT,
    GV_CLOSE_WAIT,
};

#endif /* _GAVER_H */

