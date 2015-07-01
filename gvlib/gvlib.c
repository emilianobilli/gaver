#include "gvlib.h"

int getdatasocket(socket_t *sd)
{
    return sd->so_data;
}

int getgvsocketunix(char *su_path)
{
    strcpy(su_path, "/var/gaver.sock");
    return 0;
}

/* Funciona para intercambiar mensaje con GaVer. Actios: r-->recibe, s-->envia */
int iomessage(gv_socket_t *sd ,gv_msgapi_t *msg, char action)
{
    int bytes_transfd, ret;
    char *ptr
    
    ptr = &msg;
    bytes_transfd = 0;

    if (action == 'r'){
	while(bytes_transfd < GV_MSGSIZE){
	    ret = recv(sd->so_ctrl, &ptr[bytes_transfd], GV_MSGSIZE - bytes_transfd, 0);
	    if (ret == -1)
		return -1;
	    bytes_transfd += ret;
	}
    }

    if (action == 's'){
	while(bytes_transfd < GV_MSGSIZE){
	    ret = send(sd->so_ctrl, &ptr[bytes_transfd], GV_MSGSIZE - bytes_transfd, 0);
	    if (ret == -1)
		return -1;
	    bytes_transfd += ret;
	}
    }

    return 0;
}

int gv_socket(gv_socket_t *sdgv)
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
    if (listen(sd, 5) == -1)
	return -1;
    
    /* Copio el path del socket en la estructura del socket gaver */
    sdgv->so_data = sd;
    memcpy(&(sdgv->so_addr), &addr, sizeof(struct sockaddr_un));

    return 0;
}

int gv_connect(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t len)
{
    int ret, bytes_transfd, sd_new;
    char *ptr;
    /* Creo estructura para mensaje de tipo connect y reply */
    gv_msgapi_t cntmsg, repmsg;

    cntmsg.type = GV_CONNECT_API;
    cntmsg.un.connect.ip = addr->sin_addr.s_addr;
    cntmsg.un.connect.port = GV_PORT;
    cntmsg.un.connect.vport = addr->sin_port;

    /* Para esto hay que usar memcpy() */
    memcpy(&(sd->so_addr), &cntmsg.un.connect.path, SU_PATH_SIZE);
    
    /* Envio mensaje connect por so_ctrl de gaver */
    if (iomessage(sd, &cntmsg, 's') == -1)
	return -1;

    /* Leo mensaje de reply */
    if (iomessage(sd, &repmsg, 'r') == -1)
	return -1;

    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */
    if (repmsg.type != GV_REPLY_API)
	return -1;
    if (repmsg.un.reply.code != GV_REP_CODE_OK)
	return -1;
    /* Acepto conexion de datos de Gaver con su nuevo socket descriptor */
    if ((sd_new = accept(sd->so_data, NULL, NULL )) == -1)
	return -1

    /* Cierro el viejo socket de data */
    close(sd->so_data);

    sd->so_data = sd_new;
    return 0;
}

int gv_bind(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t len)
{
    int ret, bytes_transfd;
    char *ptr;
    
    /* Creo estructura para mensaje de tipo bind y reply */
    gv_msgapi_t bindmsg, repmsg;

    
    bindmsg.type = GV_BIND_API;
    bindmsg.un.bind.ip = addr->sin_addr.s_addr;
    bindmsg.un.bind.port = GV_PORT;
    bindmsg.un.bind.vport = addr->sin_port;

    /* Envio mensaje bind por so_ctrl de gaver */
    if (iomessage(sd, &bindmsg, 's') == -1)
	return -1;

    /* Leo mensaje de reply */
    if (iomessage(sd, &repmsg, 'r') == -1)
	return -1;
    
    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */
    if (repmsg.type != GV_REPLY_API)
	return -1;
    if (repmsg.un.reply.code != GV_REP_CODE_OK)
	return -1;

    return 0;
}

int gv_listen(gv_socker_t *sd, int backlog)
{
    /* Creo estructura para mensaje de tipo listen y reply */
    gv_msgapi_t listenmsg, repmsg;

    listenmsg.type = GV_LISTEN_API;
    listenmsg.un.listen.backlog = backlog;

    /* Envio mensaje listen por so_ctrl de gaver */
    if (iomessage(sd, &listenmsg, 's') == -1)
	return -1;

    /* Leo mensaje de reply */
    if (iomessage(sd, &repmsg, 'r') == -1)
	return -1;

    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */
    if (repmsg.type != GV_REPLY_API)
	return -1;
    if (repmsg.un.reply.code != GV_REP_CODE_OK)
	return -1;

    return 0;
}

int gv_accept(gv_socker_t *sd, struct sockaddr_in* addr, socklen_t *len)
{
    /* Creo estructura para mensaje de tipo accept y reply */
    gv_msgapi_t acceptmsg, repmsg;
    
    acceptmsg.type = GV_ACCEPT_API;
    acceptmsg.un.accept.ip = addr->sin_addr.s_addr;
    acceptmsg.un.accept.port = addr->sin_port;

    /* Envio mensaje accept por so_ctrl de gaver */
    if (iomessage(sd, &acceptmsg, 's') == -1)
	return -1;

    /* Leo mensaje de reply */
    if (iomessage(sd, &repmsg, 'r') == -1)
	return -1;

    /* Verifico que el mensaje sea de tipo reply y el codigo de respuesta */
    if (repmsg.type != GV_REPLY_API)
	return -1;
    if (repmsg.un.reply.code != GV_REP_CODE_OK)
	return -1;
}

int gv_close (gv_socker_t *sd)
{
    /* Creo estructura para mensaje de tipo close */
    gv_msgapi_t closemsg;

    closemsg.type = GV_CLOSE_API;

    /* Envio mensaje close por so_ctrl de gaver */
    if (iomessage(sd, &closemsg, 's') == -1)
	return -1;

    close(sd->so_data);
    close(sd-so_ctrl);

    return 0;
}

