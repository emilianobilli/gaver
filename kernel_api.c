#include <errno.h>
#include <string.h>
#include "common.h"
#include "apitypes.h"
#include "gaver.h"
#include "sock.h"


void clean_msg(void *m)
{
    memset(m,0,sizeof(GVMSGAPISZ));
}




int (struct sock *sk, struct msg_queue *txq)

    struct gv_req_api msg;	/* Api Request  */
    struct gv_rep_api rep;	/* Api Response */

    clean_msg(&msg);
    clean_msg(&rep);
    
    if (read_msg(sk->so_local_ctrl, &msg, GVMSGAPISZ) == -1)
	return -1;

    switch (msg.msg_type)
    {
	case MSG_CONNECT:
	    /* Assign a port is not have 
	       Error is in active open */

	break;

	case MSG_BIND:

	    /*
	     *
             * !!!!!!!!!!!!! Ojo con el Network byte order 
	     *
	     */
	    /* Test if the socket have a port assigned */
	    if (sk->so_local_gvport == NO_GVPORT &&
		sk->so_state == GV_CLOSE)
	    {
		skptr = bind_gvport(sk,msg.bind.vport);
		if (skptr != NULL)
		{
		    rep.status = COMMAND_SUCCESS;
		    rep.success.addr  = local_addr;
		    rep.success.port  = local_port;
		    rep.success.vport = msg.bind.vport;
		}
		else
		{
		    rep.status = COMMAND_FAIL;
		    rep.fail.error_code = (u_int16_t) EADDRINUSE;
		}
	    }
	    else
	    {
		rep.status = COMMAND_FAIL;
		rep.fail.error_code = (u_int16_t) EINVAL;
	    }
	    if (write_msg(sk->so_local_ctrl,&rep,GVMSGAPISZ) == -1)
	    {
		/* ?? */
	    }


	break;

	case MSG_LISTEN:
	
	    /* If the socket have a port assigned put in passive open */
	    if (sk->so_local_gvport == NO_GVPORT || 
		sk->so_state != GV_CLOSE)
	    {
		rep.status = COMMAND_FAIL;
		rep.fail.error_code = (u_int16_t) EOPNOTSUPP;
	    }
	    else
	    {
		sk->so_state = GV_LISTEN;
		rep.status = COMMAND_SUCCESS;
	    }
	    if (write_msg(sk->so_local_ctrl,&rep,GVMSGAPISZ) == -1)
	    {
		/* ?? */
	    }
	    

	break;
	case MSG_ACCEPT:
	    /* Accept is in passive open */

	break;

	case MSG_CLOSE:

	break;

	default:
	    /* Protocol violation */
	break;
    }
    