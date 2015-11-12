#include "common.h"
#include "apitypes.h"





    struct gv_req_api msg;	/* Api Request  */
    struct gv_rep_api rep;	/* Api Response */

    memset(&msg,0,sizeof(struct gv_req_api);
    
    if (read_msg(sk, &msg, GVMSGAPISZ) == -1)
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

	    bzero(&rep, sizeof(struct gv_rep_api));
	    if (sock->so_local_gvport == NO_GVPORT &&
		sock->so_state == GV_CLOSE)
	    {
		skptr = bind_gvport(sock,msg.bind.vport);
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
		    rep.fail.error_code = EADDRINUSE;
		}
	    }
	    else
	    {
		rep.status = COMMAND_FAIL;
		rep.fail.error_code = EINVAL;
	    }
	    if (write_msg(sk,&rep,GVMSGAPISZ) == -1)
	    {
		/* ?? */
	    }


	break;

	case MSG_LISTEN:
	
	    /* If the socket have a port assigned put in passive open */
	    bzero(&rep, sizeof(struct gv_rep_api));

	    if (sock->so_local_gvport == NO_GVPORT || 
		sock->so_state != GV_CLOSE)
	    {
		rep.status = COMMAND_FAIL;
		rep.fail.error_code = EOPNOTSUPP;
	    }
	    else
	    {
		sock->so_state = GV_LISTEN;
		rep.status = COMMAND_SUCCESS;
	    }
	    if (write_msg(sk,&rep,GVMSGAPISZ) == -1)
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
    