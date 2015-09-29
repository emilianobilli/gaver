#ifndef GV_LIB_H
#define GV_LIB_H

#include <unistd.h>
#include <socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

/*
 * Message Types
 */
#define GV_CONNECT_API 0
#define GV_REPLY_API   1
#define GV_BIND_API    2
#define GV_LISTEN_API  3
#define GV_ACCEPT_API  4
#define GV_CLOSE_API   5

#define GV_MSG_CONNECT 0x0
#define GV_MSG_ACCEPT  0x1
#define GV_MSG_BIND    0x2
#define GV_MSG_LISTEN  0x3	
#define GV_MSG_CLOSE   0x4
#define GV_MSG_REPLY   0x5

#define GV_MSGSIZE   128
#define SU_PATH_SIZE 108

#define GV_REP_CODE_OK   0
#define GV_REP_CODE_FAIL 1

#define AF_GAVER    0x0
#define GV_PROTOCOL 0x1

#define IOMSG_ACTION_READ  0
#define IOMSG_ACTION_WRITE 1

#define GV_DATA_SD((x)) (x)->so_data
#define GV_CTRL_SD((x)) (x)->so_ctrl

typedef struct {
    int so_ctrl;			/* Control Conexion */
    int so_data;			/* Data conexion    */
    int so_status;			/* Socket Status    */
    struct sockaddr_un so_addr;		/* Unix Path 	    */
} gv_socket_t;

int gv_socket(gv_socket_t *sd, int domain, int type, int protocol);

int getgvsocketunix(char *su_path)
{
    char *gv_socket_env;
    gv_socket_env = getenv("GV_SOCKET_LOCAL");
    if ( gv_socket_env != NULL ) {
	strcpy(su_path, gv_socket_env);
	return 0;
    }
    return -1;
}

int gv_socket(gv_socket_t *sd, int domain, int type, int protocol)
{
    char gv_server_path[SU_PATH_SIZE];
    char gv_local_path[SU_PATH_SIZE];

    if (sd != NULL)
    {
	if (domain != AF_GAVER)
	    return -1;	/* EGVDOMAIN */

	if (type   != SOCK_STREAM)
	    return -1;  /* EGVTYPE   */
    
	if (protocol != GV_PROTOCOL)
	    return -1;  /* EGVPROTO  */ 
    
	if (getgvsocketunix(gv_server_path) == -1)
	    return -1;  /* EGVPATH   */

	memset(sd, 0, sizeof(gv_socket_t));

	if (tmpnam(gv_local_path) == NULL)
	    return -1;  /* errno */

	sd->so_ctrl = unix_socket_client(gv_server_path);
	if (sd->so_ctrl == -1)
	    return -1;  /* EGVSERVER && check errno */

	sd->so_data = unix_socket(gv_local_path);
	if (sd->so_data == -1)
	    return -1;  /* Check errno */
	
	strcpy(sd->so_addr.sun_path, gv_local_path);
	
	return 0;
    }
    return -1;
}


int gv_connect(gv_socket_t *sd, struct sockaddr_gv *addr, socklent_t len)
{



}

int gv_msg_connect(int socket, u_int32_t addr, u_int16_t port, u_int16_t vport, char *path)
{
    

}


typedef u_int16_t gv_port_t;

struct sockaddr_gv {
    sa_family_t    sin_family; 		/* address family: AF_INET */
    in_port_t      sin_port;   		/* port in network byte order */
    gv_port_t	   sin_gvport; 		/* port for gaver */
    struct in_addr sin_addr;   		/* internet address */
};

typedef struct {
    u_int8_t msg_type;
    union {
	struct gv_connect_msg connect;
	struct gv_reply_msg   reply;
	gv_bindapi_t    bind;
	gv_listenapi_t  listen;
	gv_accceptapi_t accept;
	gv_closesapi_t  close;
    } un;
    unsigned char res[11];
} gv_msgapi_t;


/* Mensaje de tipo Connect para la API */
struct gv_connect_msg {
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;
    u_int8_t  path[SU_PATH_SIZE];
};


/* Mensaje de tipo reply para la API */
struct gv_reply_msg {
    u_int8_t  code;
    u_int32_t ip;
    u_int16_t port;
    u_int16_t vport;
    u_int8_t  msg[118];
};


/* Mensaje de tipo bind para la API */
typedef struct {
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;
} gv_bindapi_t;


/* Mensaje de tipo listen para la API */
typedef struct {
    u_int8_t backlog;
} gv_listenapi_t;

/* Mensaje de tipo accept para la API */
typedef struct {
    u_int32_t ip;
    u_int16_t port;
    u_int16_t vport;
} gv_acceptapi_t;

/* Mensaje de tipo close para la API */
typedef struct {
    u_int8_t res[127];
} gv_closeapi_t;

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
int gv_connect(gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t len);
int gv_bind(gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t len);
int gv_listen(gv_socket_t *sd, int backlog);

/*
 * Prototipo supuesto
 *
 */
int gv_accept(gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t *len);
int gv_close (gv_socket_t *sd);

#endif /* gv_lib.h */
