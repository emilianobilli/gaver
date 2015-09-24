
#define GV_NOERROR 0x00
#define GV_OSERROR 0x01
#define GV_PROTERR 0x02
#define GV_KRNLERR 0x03

__thread int gv_errno = 0x00;

extern __thread int gv_errno;

__thread char gv_errstr[118];


void set_gverr (int gverr, int err, const char *msg, )
