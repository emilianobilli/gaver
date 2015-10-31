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
#define _CONFIGK_CODE
#include "configk.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*==================================================================================*
 * loadcfgk(): Load the command line configuration passed in argc and *argv[]   *
 *		   The function force to exit process when error ocurs		    *
 *==================================================================================*/
int loadcfgk (int argc, char *argv[], struct configk *cfg)
{
    int opt, val;
    long long int llval;
    char *endptr;
    struct servent *sent;

    memset(cfg,0,sizeof(struct configk));

    while ((opt = getopt(argc,argv, "a:p:S:s:m:M:l:")) != -1)
    {
	switch (opt)
	{
	    case 'a':
		if (!inet_aton(optarg,&(cfg->addr)))
		{
		    fprintf(stderr,"Invalid addr: %s\n", optarg);
		    return -1;
		}
		break;
	    case 'p':
		if (!strcmp("default", optarg))
		{
		    /* Check what is the default port */
		    sent = getservbyname("gaver", "udp");
		    if (sent != NULL)
			cfg->port = sent->s_port;
		    else
		    {
			fprintf(stderr, "Service gaver not found\n");
			return -1;
		    }
		}
		else
		{ 
		    errno = 0;
		    val = strtol(optarg,&endptr,10);

		    if ((errno == ERANGE && 
		        (val == LONG_MAX || val == LONG_MIN)) ||
                	(errno != 0 && val == 0)) 
		    {
            		perror("port");
            		return -1;
		    }
		    if ( val > 65535 || val <= 0 )
		    {
			fprintf(stderr,"Config: Port out of range\n");
			return -1;
		    }
		    cfg->port = htons(val);
		}
		break;
	    case 'S':
		    errno = 0;
		    llval = strtoll(optarg,&endptr, 10);
		    
		    if ((errno == ERANGE && 
		        (val == LLONG_MAX || val == LLONG_MIN)) ||
                	(errno != 0 && val == 0)) 
		    {
			perror("socket_bps");
			return -1;
		    }
		    cfg->socket_bps = (u_int64_t) llval;
		break;
	    case 's':
		    errno = 0;
		    llval = strtoll(optarg,&endptr, 10);
		    
		    if ((errno == ERANGE && 
		        (val == LLONG_MAX || val == LLONG_MIN)) ||
                	(errno != 0 && val == 0)) 
		    {
			perror("overal_bps");
			return -1;
		    }
		    cfg->overal_bps = (u_int64_t) llval;

		break;
	    case 'm':
		errno = 0;
		val = strtol(optarg,&endptr,10);
		if ((errno == ERANGE && 
		    (val == LONG_MAX || val == LONG_MIN)) ||
            	    (errno != 0 && val == 0)) 
		{
            	    perror("mtu");
            	    return -1;
		}
		if ( val > 9216 || val <= 0 )
		{
		    fprintf(stderr,"Config: Mtu out of range");
		    return -1;
		}
		cfg->mtu = val;
		break;
	    case 'M':
		break;
	    case 'l':
		if (strlen(optarg) >= UNIX_PATH_MAX)
		{
		    fprintf(stderr, "Listen Socket Name Out of Range\n");
		    return -1;
		}
		strcpy(cfg->listen_api, optarg);
		break;
	    default:
		break;
	}
    }
    return 0;
}



void dumpcfgk (FILE *f, struct configk *cfg)
{
    fprintf(f, "Addr: %s\nPort: %d\nUnix Socket Api: %s\nMtu:  %d\nOveral Bps: %ld\nSocket Bps: %ld\n", 
		inet_ntoa(cfg->addr),
		ntohs(cfg->port),
		cfg->listen_api,
		cfg->mtu,
		cfg->overal_bps,
		cfg->socket_bps);
    return;		
}

