#include <errno.h>
#include <sys/types.h>
#include "gvlib.h"


/*
 *	Author: Nicolas Pajoni
 */

int getdatasocket(socket_t *sd)
{
    return sd->so_data;
}

int getctrlsocket(socket_t *sd)
{
    return sd->so_ctrl;
}

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

ssize_t iomessage(int sd, gv_msgapi_t *msg, int action)
{
    ssize_t bytes_transfd, ret;
    u_int8_t *ptr;
    
    ptr = (u_int8_t *) &msg;
    bytes_transfd = 0;

    if (action == IOMSG_ACTION_READ )
    {
	while(bytes_transfd < GV_MSGSIZE)
	{
	    ret = recv(sd, &ptr[bytes_transfd], GV_MSGSIZE - bytes_transfd, 0);
	    if (ret == -1) {
		if (errno != EINTR)
		/*
		 * Os error
		 */
		    return -1;
		else
		    continue;
	    }
	    bytes_transfd += ret;
	}
    }

    if (action == IOMSG_ACTION_WRITE)
    {
	while(bytes_transfd < GV_MSGSIZE)
	{
	    ret = send(sd, &ptr[bytes_transfd], GV_MSGSIZE - bytes_transfd, 0);
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

int gv_socket(gv_socket_t *sdgv, int domain, int type, int protocol)
{
    int sd;
    struct sockaddr_un addr;
    
    /* Inicializo la estructura del socket con el valor cero */
    memset(&addr, 0, sizeof(struct sockaddr_un));

    /* Asigno valores de tipo y path al socket unix */
    addr.sun_family = AF_UNIX;
    if (getgvsocketunix(addr.sun_path) != 0 )
	return -1;

    /* Creo el socket de control para conectarme a Gaver  */
    if (sd = socket(PF_UNIX, SOCK_STREAM, 0) == -1)
	return -1;

    /* Me conecto al socket */
    if (connect (sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
	return -1;

    sdgv->so_ctrl = sd;
    
    /* Creo el socket del server */
    if (sd = socket(PF_UNIX, SOCK_STREAM, 0) == -1)
	return -1;

    /* Genero un path aleatorio para el socket unix */
    sprintf (addr.sun_path, "%ld-%d-%d.sock", time(), getpid(), rand()%100);

    /* Realizo un bind */
    if (bind(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
	return -1;

    /* Pongo el socket en modo listen  */
    if (listen(sd, 1) == -1)
	return -1;
    
    /* Copio el path del socket en la estructura del socket gaver */
    sdgv->so_data = sd;
    memcpy(&(sdgv->so_addr), &addr, sizeof(struct sockaddr_un));

    return 0;
}


int gv_connect_util (int sd, const char *sun_path, struct sockaddr_gv *addr, socklen_t len)
{
    /* Creo estructura para mensaje de tipo connect y reply */
    u_int8_t msg[GV_MSGSIZE];
    gv_msgapi_t *cntmsg, *repmsg;
    int sd_new;

    cntmsg = (gv_msgapi_t *) &msg[0];
    cntmsg->type = GV_CONNECT_API;
    cntmsg->un.connect.ip    = addr->sin_addr.s_addr;
    cntmsg->un.connect.port  = addr->sin_port;
    cntmsg->un.connect.vport = addr->sin_gvport;

    /* Para esto hay que usar memcpy() */
    memcpy(cntmsg->un.connect.path, sun_path, SU_PATH_SIZE);
    
    /* Envio mensaje connect por so_ctrl de gaver */
    if (iomessage(sd->so_ctrl, msg, IOMSG_ACTION_WRITE) == -1)
    {
	gv_errno = GV_OSERROR;
	return -1;
    }

    /* Leo mensaje de reply */
    if (iomessage(sd->so_ctrl, msg, IOMSG_ACTION_READ) == -1)
    {
	gv_errno = GV_OSERROR;
	return -1;
    }

    repmsg = (gv_msgapi_t *) &msg[0];

    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */
    if (repmsg->type != GV_REPLY_API)
    {
	gv_errno = GV_PROTERR;
	return -1;
    }
    
    if (repmsg->un.reply.code != GV_REP_CODE_OK)
    {
	gv_errno = GV_KRNLERR;
	memcpy(gv_errstr, repmsg->un.reply.msg);
	return -1;
    }
    return 0;
}


int gv_connect(gv_socket_t *sd, struct sockaddr_gv *addr, socklen_t len)
{
    int sd_new;

    if (gv_connect_util(sd->so_ctrl, sd->sun_path, addr, len) == -1)
	return -1;

    if ((sd_new = accept(sd->so_data, NULL, NULL )) == -1) {
	gv_errno = GV_OSERROR;
	return -1
    }
    /* Cierro el viejo socket de data */
    close(sd->so_data);
    sd->so_data = sd_new;

    gv_errno = GV_NOERROR;
    return 0;
}

int gv_bind(gv_socket_t *sd, struct sockaddr_gv* addr, socklen_t len)
{
    /* Creo estructura para mensaje de tipo bind y reply */
    u_int8_t msg[GV_MSGSIZE];
    gv_msgapi_t *bindmsg, *replymsg;
    
    bindmag = (gv_msgapi_t *)&msg[0];

    bindmsg->type = GV_BIND_API;
    bindmsg->un.bind.ip    = addr->sin_addr.s_addr;
    bindmsg->un.bind.port  = addr->sin_port;
    bindmsg->un.bind.vport = addr->sin_gvport;

    /* Envio mensaje bind por so_ctrl de gaver */
    if (iomessage(sd->so_ctrl, msg, IOMSG_ACTION_WRITE) == -1)
	return -1;

    /* Leo mensaje de reply */
    if (iomessage(sd->so_ctrl, msg, IOMSG_ACTION_READ) == -1)
	return -1;
    
    replymsg = (gv_msgapi_t *)&msg[0];
    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */

    /*
     * Definir tipos de mensaje de error
     */
    if (repmsg->type != GV_REPLY_API)
	return -1;
    if (repmsg->un.reply.code != GV_REP_CODE_OK)
	return -1;

    return 0;
}

int gv_listen(gv_socker_t *sd, int backlog)
{
    u_int8_t msg[GV_MSGSIZE];
    /* Creo estructura para mensaje de tipo listen y reply */
    gv_msgapi_t *listenmsg, *repmsg;

    listenmsg = (gv_msgapi_t *) &msg[0];
    listenmsg->type = GV_LISTEN_API;
    listenmsg->un.listen.backlog = backlog;

    /* Envio mensaje listen por so_ctrl de gaver */
    if (iomessage(sd->so_ctrl, msg, IOMSG_ACTION_WRITE) == -1)
	return -1;

    /* Leo mensaje de reply */
    if (iomessage(sd->so_ctrl, msg, IOMSG_ACTION_READ) == -1)
	return -1;

    repmsg = (gv_msgapi_t *) &msg[0];

    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */
    if (repmsg->type != GV_REPLY_API)
	return -1;
    if (repmsg->un.reply.code != GV_REP_CODE_OK)
	return -1;

    return 0;
}

/*
 *int gv_accept(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t *len)
  {
    Creo estructura para mensaje de tipo accept y reply 
    gv_msgapi_t *acceptmsg, *repmsg;


    acceptmsg->type = GV_ACCEPT_API;
    acceptmsg->un.accept.ip = addr->sin_addr.s_addr;
    acceptmsg->un.accept.port = addr->sin_port;

    /* Envio mensaje accept por so_ctrl de gaver 
    if (iomessage(sd, &acceptmsg, 's') == -1)
	return -1;

    /* Leo mensaje de reply 
    if (iomessage(sd, &repmsg, 'r') == -1)
	return -1;

    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta 
    if (repmsg.type != GV_REPLY_API)
	return -1;
    if (repmsg.un.reply.code != GV_REP_CODE_OK)
	return -1;
}
*/
int gv_close (gv_socker_t *sd)
{
    /* Creo estructura para mensaje de tipo close */
    gv_msgapi_t closemsg;

    closemsg.type = GV_CLOSE_API;

    /* Envio mensaje close por so_ctrl de gaver */
    if (iomessage(sd, &closemsg, 's') == -1)
	return -1;

    close(sd->so_data);
    close(sd->so_ctrl);

    return 0;
}


