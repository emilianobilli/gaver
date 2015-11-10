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
#include <linux/limits.h>
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


void usage(void)
{
    int i;
    char *usage_str = "\nUsage:\n gvd [config parameters]\n\nConfig Parameters:\n";
    char *cfg[] = { " -a <addr>\t\tIp Address\n",
		    " -p <port>\t\tPort\n",
		    " -S <mbps>\t\tOveral Speed measured in bit per seconds\n",
		    " -s <mbps>\t\tSocket Speed: Maximun per socket speed measured in bit per seconds\n",
		    " -r <bytes>\t\tRead Max Buffer Per Socket\n",
		    " -w <byets>\t\tWrite Max Buffer Per Socket\n",
		    " -m <bytes>\t\tMtu Size\n",
		    " -l <path>\t\tUnix Socket Api\n",
		    " -P <path>\t\tPid File\n",
		    " -h       \t\tShow This Help\n",
		    NULL };
    char *end = "\nReport Bugs to: <emiliano.billi@gmail.com>\n";
    printf("%s", usage_str);
    
    for ( i = 0; cfg[i]; i++ )
	printf("%s",cfg[i]);

    printf("%s", end);
    return;
}


/*======================================================================================*
 * loadcfgk(): Load the command line configuration passed in argc and *argv[]		*
 *		   The function force to exit process when error ocurs			*
 *======================================================================================*/
int loadcfgk (int argc, char *argv[], struct configk *cfg)
{
    int opt, val;
    long long int llval;
    char *endptr;
    struct servent *sent;

    memset(cfg,0,sizeof(struct configk));

    while ((opt = getopt(argc,argv, "a:p:P:S:s:m:l:r:w:h")) != -1)
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
	case 'r':
	    errno = 0;
	    llval = strtoll(optarg,&endptr, 10);
	    
	    if ((errno == ERANGE && 
	        (val == LONG_MAX || val == LONG_MIN)) ||
            	(errno != 0 && val == 0)) 
	    {
		perror("rmem");
		return -1;
	    }
	    cfg->rmem = (u_int64_t) llval;
	    break;

	case 'w':
	    errno = 0;
	    llval = strtoll(optarg,&endptr, 10);
	    
	    if ((errno == ERANGE && 
	        (val == LONG_MAX || val == LONG_MIN)) ||
            	(errno != 0 && val == 0)) 
	        {
	    	    perror("wmem");
		    return -1;
		}
	        cfg->wmem = (u_int32_t) llval;
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

	case 'h':
	    return -1;
	    break;
	case 'l':
	    if (strlen(optarg) >= UNIX_PATH_MAX)
	    {
	        fprintf(stderr, "Listen Socket Name Out of Range\n");
	        return -1;
	    }
	    strcpy(cfg->listen_api, optarg);
	    break;
	case 'P':
	    if (strlen(optarg) >= NAME_MAX)
	    {
	        fprintf(stderr, "Pid file Out of Range\n");
	        return -1;
	    }
	    strcpy(cfg->pid_file, optarg);
	    break;
	default:
	    return -1;
	    break;
	}
    }
    if(!cfg->port)
    {
	fprintf(stderr,"Config: Port number is mandatory\n");
	return -1;
    }
    if(!strlen(cfg->listen_api))
    {
	fprintf(stderr,"Config: Listen Api is mandatory\n");
	return -1;
    }
    if (!cfg->overal_bps)
    {
	fprintf(stderr,"Config: Overal Speed is mandatory\n");
	return -1;
    }
    if (!cfg->pid_file)
    {
	fprintf(stderr,"Config: Pid file is mandatory\n");
	return -1;
    }
    if (!cfg->mtu)
    {
	fprintf(stderr,"Config: Mtu is mandatory\n");
	return -1;
    }

    return 0;
}



void dumpcfgk (FILE *f, struct configk *cfg)
{
    fprintf(f, "GaVer\n=====\nAddr: %s\nPort: %d\nUnix Socket Api: %s\nMtu:  %d\nOveral Bps: %ld\nSocket Bps: %ld\nRead Memory: %u\nWrite Memory: %u\nPid File: %s\n", 
		inet_ntoa(cfg->addr),
		ntohs(cfg->port),
		cfg->listen_api,
		cfg->mtu,
		cfg->overal_bps,
		cfg->socket_bps,
		cfg->rmem,
		cfg->wmem,
		cfg->pid_file);
    return;		
}

