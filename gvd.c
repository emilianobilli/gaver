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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "configk.h"
#include "heap.h"
#include "itc.h"
#include "sockopt.h"
#include "kernel.h"
#include "util.h"
#include "glo.h"

int writepid (const char *pidfile)
{
    char buff[20];
    pid_t pid = getpid();
    int fd;

    fd = open(pidfile, O_CREAT | O_WRONLY, S_IRWXU);
    if (fd == -1)
	return -1;

    sprintf(buff,"%d", pid);
    if (write(fd, buff, strlen(buff)) == -1)
    {
	close(fd);
	return -1;
    }
    close(fd);
    return 0;
}


int main (int argc, char *argv[])
{
    struct timespec itp;
    struct timespec rt;
    int    err = -1;
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
    api_socket = unix_socket(gvcfg.listen_api);		/* api_socket -> Global Global Socket */
    if (api_socket == -1)
    {
	PANIC(errno, "gvd", "api_socket");
	exit(EXIT_FAILURE);
    }

    errno = 0;
    netstat_socket = unix_socket(gvcfg.netstat);	/* netstat_socket -> Global Socket */
    if (netstat_socket == -1)
    {
	close(api_socket);
	PANIC(errno, "gvd", "netstat");
	exit(EXIT_FAILURE);
    }

    /*
     * Create the UDP Socket for Gaver
     */
    errno = 0;
    ifudp = ipv4_udp_socket_nbo(gvcfg.addr.s_addr,gvcfg.port);	/* ifudp -> Global Socket */
    if (ifudp == -1)
    {
	close(api_socket);
	close(netstat_socket);
	PANIC(errno, "gvd", "ipv4_udp_socket_nbo");
	exit(EXIT_FAILURE);
    }

    local_addr = gvcfg.addr.s_addr;	/* Global */
    local_port = gvcfg.port;		/* Global */

    /* Init the Heap */
    init_heap();

    /* Init ITC */
    errno = 0;
    itc_init();
    itc_block_signal(); 
    itc_event = itc_signalfd_init();		/* Global */
    if (itc_event == -1) 
    {
	close(netstat_socket);
	close(api_socket);
	close(ifudp);
	PANIC(errno,"gvd","itc_signalfd_init");
	exit(EXIT_FAILURE);
    }

    /* Inter Packet Time */
    errno = 0;
    itp.tv_sec  = 0;
    itp.tv_nsec = pktime(gvcfg.overal_bps, gvcfg.mtu);

    ipkt =  ttod(&itp);			/* Global */

    output_timer = event_timer(&itp);		/* Global Timer fd */
    if (output_timer == -1) 
    {
	close(api_socket);
	close(netstat_socket);
	close(ifudp);
	close(itc_event);
	PANIC(errno, "gvd","event_timer(itp)");
	exit(EXIT_FAILURE);
    }

    /* Refresh Time      */
    errno = 0;
    refresh = getreftime(gvcfg.overal_bps, gvcfg.mtu);

    rft = refresh;				/* Global */

    dtot(&refresh,&rt);
    refresh_timer = event_timer(&rt);		/* Global Timer fd */

    if (refresh_timer == -1) {
	close(api_socket);
	close(ifudp);
	close(netstat_socket);
	close(itc_event);
	close(output_timer);
	PANIC(errno, "gvd", "event_timer(&rt)");
	exit(EXIT_FAILURE);
    }

    errno = 0;
    if (gvcfg.error) {
        err = open(gvcfg.error, O_CREAT | O_WRONLY, S_IRWXU);
	if (err == -1) {
	    close(api_socket);
	    close(ifudp);
	    close(itc_event);
	    close(output_timer);
	    close(refresh_timer);
	    close(netstat_socket);
	    PANIC(errno,"gvd","daemon");
	    exit(EXIT_FAILURE);
	}
    }
    /*
     * This function daemonize the process
     */
    errno = 0;
    umask(S_IWGRP | S_IWOTH);
/*    if (daemon(0,0)==-1)
    {
	close(api_socket);
	close(ifudp);
	close(itc_event);
	close(output_timer);
	close(netstat_socket);
	close(refresh_timer);
	PANIC(errno,"gvd","daemon");
	exit(EXIT_FAILURE);
    }
*/

    if (err != -1) {
	dup2(err,STDERR_FILENO);
	close(err);
    }
	    
    errno = 0;
    if (writepid(gvcfg.pid_file)==-1)
    {
	close(api_socket);
	close(ifudp);
	close(itc_event);
	close(output_timer);
	close(netstat_socket);
	close(refresh_timer);
	PANIC(errno,"gvd","writepid");
	exit(EXIT_FAILURE);
    }    

    /* 
     * Finaly dispach the kernel
     */

    kernel(NULL);

    while (1)
	sleep(5);

    return 0;
}

