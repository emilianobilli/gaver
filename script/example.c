#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


[GaVer]
addr="";
port="-1";
schedule="";
overal_speed="";
mtu_limit="1500";
max_speed="";

[Api]
listen="lala";


-a 0.0.0.0 -p default -S fifo -s 1000000000 -m 1500 -M 200000


struct configk {
    struct in_addr addr; /* Network bit order addr */
    u_int16_t      port; /* Network bit order port */
    char 	   listen_api[108];
    int sched;
    int mtu;
    u_int64_t overal_bps; /* Velocidad Maxima */	
    u_int64_t socket_bps; /* Velocidad Maxima por socket */
}


int load_configk (int argc, char *argv[], struct configk *cfg)
{
    int opt, val;
    char *endptr;
    struct servent *sent;

    while ((opt = getopt(argc,argv, "a:p:S:s:m:M:d")) != -1)
    {
	switch (opt)
	{
	    case 'a':
		if (!inet_aton(optarg,cfg->addr))
		{
		    perror("inet_aton");
		    exit(EXIT_FAILURE);
		}
		break;
	    case 'p':
		if (strcmp("default", optarg))
		{
		    /* Check what is the default port */
		    sent = getservbyname("gaver", "udp");
		    if (sent != NULL)
			cfg->port = htons(sent->s_port);
		    else
		    {
			fprintf(stderr, "Service gaver not found\n");
			exit(EXIT_FAILURE);
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
            		perror("strtol");
            		exit(EXIT_FAILURE);
		    }
		    if ( val > 65535 || val <= 0 )
		    {
			fprintf(stderr,"Config: Invalid Port");
			exit(EXIT_FAILURE);
		    }
		    cfg->port = htons(val);
		}
		break;
	    case 'S':
	    case 's':
	    case 'm':
	    case 'M':
	    case 'd':
	}
    }
}


int
       main(int argc, char *argv[])
       {
           int flags, opt;
           int nsecs, tfnd;

           nsecs = 0;
           tfnd = 0;
           flags = 0;
           while ((opt = getopt(argc, argv, "nt:")) != -1) {
               switch (opt) {
               case 'n':
                   flags = 1;
                   break;
               case 't':
                   nsecs = atoi(optarg);
                   tfnd = 1;
                   break;
               default: /* '?' */
                   fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                           argv[0]);
                   exit(EXIT_FAILURE);
               }
           }

