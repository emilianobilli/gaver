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


#include <errno.h>
#include <string.h>
#include "common.h"
#include "mbuff.h"
#include "glo.h"
#include "apitypes.h"
#include "gaver.h"
#include "sock.h"


int do_socket_request(struct sock *sk, struct msg_queue *txq)
{
    struct gv_req_api msg;	/* Api Request  */
    struct gv_rep_api rep;	/* Api Response */
    struct sock *skptr;		/* Pointer to Sock Struct */

    memset(&msg, 0,sizeof(GVMSGAPISZ) );
    memset(&rep, 0,sizeof(GVMSGAPISZ) );
    
    if (read_msg(sk->so_loctrl, &msg, GVMSGAPISZ) == -1)
	return -1;

    switch (msg.msg_type)
    {
	case MSG_CONNECT:
	    if (sk->so_state != GV_CLOSE || sk->so_loctrl_state != CTRL_NONE)
	    {
		/*
		 * Esto es un error en el orden de las llamadas a las 
		 * funciones, se supone que se esta realizando una llamada
		 * a connect con un socket ya conectado, por consiguiente
		 * falla
		 */
		rep.status = COMMAND_FAIL;
		rep.un.fail.error_code = (u_int16_t) EOPNOTSUPP;

		if (write_msg(sk->so_loctrl,&rep,GVMSGAPISZ) == -1)
		    return -1;
	    }

	    if (sk->so_local_gvport == NO_GVPORT)
		/*
		 * El socket no esta ligado a ningun puerto
		 * ya que no se realizo de forma explicita con 
		 * la llamada a funcion bind(), por 
		 * consiguiente se busca un puerto libre y se
		 * lo liga, esta llamada a funcion no deberia
	         * fallar ya que hay 2^16 puertos disponibles y
		 * solamtente 255 sockets.
		 */
		bind_free_gvport(sk);
	
	    /*
	     * Generar Mensaje y cambiar status
             *
	     */

	    /* sk->so_state = GV_CONNECT_SENT; */

	    /* Assign a port is not have 
	       Error is in active open */

	break;

	case MSG_BIND:

	    /*
                 !!!!!!!! Ojo con el network byte Order 
	    */

	    /* Test if the socket have a port assigned */
	    if (sk->so_local_gvport == NO_GVPORT &&
		sk->so_state == GV_CLOSE)
	    {
		skptr = bind_gvport(sk,msg.un.bind.vport);
		if (skptr != NULL)
		{
		    rep.status = COMMAND_SUCCESS;
		    rep.un.success.addr  = local_addr;
		    rep.un.success.port  = local_port;
		    rep.un.success.vport = msg.un.bind.vport;
		}
		else
		{
		    rep.status = COMMAND_FAIL;
		    rep.un.fail.error_code = (u_int16_t) EADDRINUSE;
		}
	    }
	    else
	    {
		rep.status = COMMAND_FAIL;
		rep.un.fail.error_code = (u_int16_t) EINVAL;
	    }
	    if (write_msg(sk->so_loctrl,&rep,GVMSGAPISZ) == -1)
		return -1;

	break;

	case MSG_LISTEN:
	    /* If the socket have a port assigned put in passive open */
	    if (sk->so_local_gvport == NO_GVPORT || 
		sk->so_state != GV_CLOSE)
	    {
		rep.status = COMMAND_FAIL;
		rep.un.fail.error_code = (u_int16_t) EOPNOTSUPP;
	    }
	    else
	    {
		sk->so_state = GV_LISTEN;
		rep.status = COMMAND_SUCCESS;
	    }
	    if (write_msg(sk->so_loctrl,&rep,GVMSGAPISZ) == -1)
		return -1;
	
	break;

	case MSG_ACCEPT:
	    /* Accept is in passive open */
    
	    if (sk->so_state == GV_LISTEN) 
		/*
		 * En esta etapa, la capa superior pide 
		 * aceptar una conexion, pero todavia no llego
		 * ninguna solicitud por parte del Par, por 
	         * consiguiente el proceso de la capa superior 
		 * se bloquea
		 */
		sk->so_loctrl_state = CTRL_ACCEPT_REQ;

	    else if (sk->so_state == GV_CONNECT_RCVD)
	    {
		/*
		 * En esta etapa ya habia llegado un intento de conexion
		 * pero todavia la capa superior no habia aceptado.
		 * Al aceptar el mensaje, se debe enviar un accept
		 */
		sk->so_loctrl_state = CTRL_ACCEPT_PEND;    
	    }
	    else
	    {
		rep.status = COMMAND_FAIL;
		rep.un.fail.error_code = (u_int16_t) EOPNOTSUPP;
	    } 

	break;

	case MSG_CLOSE:

	break;

	default:
	    /* Protocol violation */
	break;
    }
    return 0;
}    