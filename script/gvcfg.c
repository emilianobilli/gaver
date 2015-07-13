[GaVer]
addr="";
port="-1";
schedule="";
overal_speed="";
mtu_limit="1500";
max_speed="";

[Api]
listen="lala";

[Memory]
mem="258212312";

-a 0.0.0.0 -p default -S fifo -s 1000000000 -m 1500 -M 200000


struct {
    struct in_addr addr;
    u_int16_t	   port;
    int		   sched;
    u_int64_t	   bps_seed;
    int		   mtu;
    u_int64_t	   max_seed;
} gv_cfg;

int gv_error
char *gv_error_str[] = { "Invalid address",
			 "Invalid port",
			 "Unknow parameter for schedule option",
			 "Overal speed must be a number",
			 "MTU value must be a number",
			 "Max speed per conection must be a number" };

#define EINVADDR  0
#define EINVPORT  1
#define EUNKPASH  2
#define EOVSPEED  3
#define EMTUNUMB  4
#define EMAXSPEED 5


int load_gvcgf (int argc, char *argv[])
{
    int opt;


    while (( opt = getopt(argc,argv,"a:p:S:s:m:M:")) != -1)
    {
	switch(opt) {
	case 'a':
	    if (inet_aton(optarg, &(gv_cfg.addr)== 0)) {
		gv_error = EINVADDR;
	    	return -1;
	    }
	    break;
	case 'p':
	    break;
	case 'S':
	    break;
	case 's':
	    break;
	case 'm':
	    break;
	case 'M':
	    break;
	default:
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

