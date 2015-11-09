#include <unistd.h>
#include <stdio.h>
#include <errno.h>


#include "configk.h"
#include "heap.h"
#include "itc.h"
#include "sockopt.h"
#include "util.h"
#include "glo.h"



int main (int argc, char *argv[])
{
    struct timespec itp;
    struct timespec rt;
    double refresh;

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
	close(api_socket);
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
    if (itc_event == -1) 
    {
	close(api_socket);
	close(ifudp);
	perror("itc_signalfd_init");
	exit(EXIT_FAILURE);
    }

    /* Inter Packet Time */
    errno = 0;
    itp.tv_sec  = 0;
    itp.tv_nsec = pktime(gvcfg.overal_bps, gvcfg.mtu);

    fprintf(stderr, "Inter Packet Time: %lf Sec\n", ttod(&itp));

    output_timer = event_timer(&itp); 
    if (output_timer == -1) 
    {
	close(api_socket);
	close(ifudp);
	close(itc_event);
	perror("event_timer(itp)");
	exit(EXIT_FAILURE);
    }

    /* Refresh Time      */
    errno = 0;
    refresh = getreftime(gvcfg.overal_bps, gvcfg.mtu);

    fprintf(stderr,"Refresh Time: %lf Sec\n", refresh);
    dtot(&refresh,&rt);
    refresh_timer = event_timer(&rt);

    if (refresh_timer == -1)
    {
	close(api_socket);
	close(ifudp);
	close(itc_event);
	close(output_timer);
	perror("event_timer(&rt)");
	exit(EXIT_FAILURE);
    }

    return 0;
}