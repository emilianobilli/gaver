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

#include <types.h>

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
};

#define DATA		0x00
#define SYN		0x01
#define ACK		0x02
#define NACK		0x04
#define CONNECT		0x08
#define FINISH		0x10
#define ACCEPT		0x20
#define WAIT_REPLY	0x40
#define REPLY		0x80

#define SYN_ACK       (SYN | ACK)
#define SYN_ACK_NACK  (SYN | ACK | NACK)


#define SPEED_BIT	0x00
#define SPEED_KBIT	0x01
#define SPEED_MBIT	0x02
#define SPEED_GBIT	0x03

/*
 * GaVer Syn Payload
 */
struct gvsyn {
    u_int64_t ts_sec;
    u_int64_t ts_nsec;
    u_int32_t recv_window;
    u_int16_t current_speed;
    u_int16_t reserved;
    u_int64_t seq_ack;
};

#define GET_SPEEDUNIT(x) ( ( (x) & 0xC000 ) >> 14 )
#define GET_SPEEDVAL(x)  ( (x) & 0x3FFF )


#define SPEED_FAIR	0x00
#define SPEED_FIX	0x01


/*
 * GaVer Connect Payload
 */
struct gvconnect {
    u_int16_t upload_speed;
    u_int16_t download_speed;
    u_int32_t recv_window;
    u_int64_t data_seq;
    u_int16_t mtu;
    u_int8_t  speed_type;
    u_int8_t  cnt_number;
    u_int32_t reserved;
    u_int64_t ts_sec;
    u_int64_t ts_nsec;
};

/*
 * GaVer Accept Payload
 */
struct gvaccept {
    u_int16_t upload_speed;
    u_int16_t download_speed;
    u_int32_t recv_window;
    u_int64_t data_seq;
    u_int16_t mtu;
    u_int8_t  speed_type;
    u_int8_t  cnt_number;
    u_int32_t reserved;
    u_int64_t host_ts_sec;
    u_int64_t host_ts_nsec;
    u_int64_t ts_sec;
    u_int64_t ts_nsec;
};

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
    GV_CLOSE,
    GV_LISTEN,
    GV_FINISH_SENT,
    GV_CLOSE_WAIT,
};


#define IP_HDRSIZE 20
#define IP_OPTSIZE 40
#define UDP_HDRSIZE 8

#define DFL_MTU 1500
#define PAYLOAD_SIZE (FL_MTU-IP_HDRSIZE-IP_OPTSIZE-UDP_HDRSIZE)

#endif /* _GAVER_H */

