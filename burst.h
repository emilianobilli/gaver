#ifndef _BURST_H
#define _BURST_H

#include "types.h"

/*
 * Macros to conver from gbps, mbps and kbsp to bps
 */

#define GBPS_TOBPS(x) ((x) * 1000000000)
#define MBPS_TOBPS(x) ((x) * 1000000)
#define KBPS_TOBPS(x) ((x) * 1000)


EXTERN long pksec (long bandwith, int mtu);

/*
 * Return the necesaty time in ns between one packet and other to keep the speed
 */
EXTERN long pktime (long bandwith, int mtu);


#endif

