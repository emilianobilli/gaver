#ifndef GV_LIB_H
#define GV_LIB_H

#include <unistd.h>
#include <socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define GV_CONNECT_API 0
#define GV_REPLY_API 1
#define GV_BIND_API 2
#define GV_LISTEN_API 3
#define GV_ACCEPT_API 4
#define GV_CLOSE_API 5

#define GV_PORT 55000
#define GV_MSGSIZE 128
#define SU_PATH_SIZE 108

#define GV_REP_CODE_OK 0
#define GV_REP_CODE_FAIL 1

typedef struct {
    int so_ctrl;
    int so_data;
    struct sockaddr_un so_addr;
} gv_socket_t;


typedef struct {
    u_int8_t type;
    union {
	gv_cntapi_t connect;
	gv_replyapi_t reply;
	gv_bindapi_t bind;
	gv_listenapi_t listen;
	gv_acptapi_t accept;
	gv_clsapi_t close;
    } un;
} gv_msgapi_t;

/* Mensaje de tipo Connect para la API */
typedef struct {
    u_int32_t ip;
    u_int16_t port;
    u_int16_t vport;
    u_int8_t path[108];
    u_int8_t res[11];
} gv_cntapi_t;

/* Mensaje de tipo reply para la API */
typedef struct {
    u_int8_t code;
    u_int32_t ip;
    u_int16_t port;
    u_int16_t vport;
    u_int8_t msg[118];
} gv_replyapi_t;

/* Mensaje de tipo bind para la API */
typedef struct {
    u_int32_t ip;
    u_int16_t port;
    u_int16_t vport;
    u_int8_t res[119];
} gv_bindapi_t;

/* Mensaje de tipo listen para la API */
typedef struct {
    u_int8_t backlog;
    u_int8_t res[126];
} gv_listenapi_t;

/* Mensaje de tipo accept para la API */
typedef struct {
    u_int32_t ip;
    u_int16_t port;
    u_int8_t res[121];
} gv_acptapi_t;

/* Mensaje de tipo close para la API */
typedef struct {
    u_int8_t res[127];
} gv_clsapi_t;

int getdatasocket(socket_t *sd);

int getctrlsocket(socket_t *sd);

int getgvsocketunix(char *su_path);

/*
 * Prototipo supuesto
 *
 * domain: AF_INET
 * type: SOCK_STREAM
 * protocol: 0
 */
int gv_socket(gv_socket_t *sd, int domain, int type, int protocol);

/*
gv_socket_t sock;

ret = gv_socket(&sock, AF_INET, SOCK_STREAM, 0);
if (ret != -1) {
    gv_bind
    gv_listen
    gv_accept
}

*/
int gv_socket(gv_socket_t *sd);
int gv_connect(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t len);
int gv_bind(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t len);
int gv_listen(gv_socker_t *sd, int backlog);

/*
 * Prototipo supuesto
 *
 */
int gv_accept(gv_socket_t *sd, gv_socket_t *unused, struct sockaddr *addr, socklen_t *len);
/*
gv_socket_t sock;
ret = gv_accept(&sock, NULL,(struct sockaddr *) &addr, &len);
*/


int gv_accept(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t *len);
int gv_close (gv_socker_t *sd);

#endif /* gv_lib.h */
