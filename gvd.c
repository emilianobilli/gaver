#include <unistd.h>
#include <stdio.h>
#include <errno.h>


#include "configk.h"
#include "heap.h"
#include "itc.h"
#include "sockopt.h"
#include "glo.h"



int main (int argc, char *argv[])
{

    if (argc == 1)
    {
	usage();
	exit(EXIT_FAILURE);
    }

    if (loadcfgk(argc, argv, &gvcfg) == -1)
    {
	usage();
	exit(EXIT_FAILURE);
    }

    dumpcfgk(stdout,&gvcfg);

    /*
     * Create the Unix Socket for the API
     */
    errno = 0;
    api_socket = unix_socket(gvcfg.listen_api);
    if (api_socket == -1)
    {
	perror("socket_unix");
	exit(EXIT_FAILURE);
    }

    /*
     * Create the UDP Socket for Gaver
     */
    errno = 0;
    ifudp = ipv4_udp_socket_nbo(gvcfg.addr.s_addr,gvcfg.port);
    if (ifudp == -1)
    {
	perror("ipv4_udp_socket_nbo");
	exit(EXIT_FAILURE);
    }

    /* Init the Heap */
    init_heap();

    /* Init ITC */
    errno = 0;
    itc_init();
    itc_block_signal(); 
    itc_event = itc_signalfd_init();
    if (itc_event == -1) {
	perror("itc_signalfd_init");
	exit(EXIT_FAILURE);
    }


    return 0;
}