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

#ifndef _DEFS_H
#define _DEFS_H

#define REAL_SPEED		95	/* Value of the real speed in % */
#define START_CAPWIN		100	/* Value of start Congestion Avoidance Window */

#define START_TIMEOUT_SEC	120	/* Measured in seconds */
#define START_TIMEOUT_NSEC	0	

#define START_DATASEQ		1000
#define START_CTRLSEQ		2000

#define IP_HDRSIZE		20
#define IP_OPTSIZE		40
#define UDP_HDRSIZE		8

#define DFL_MTU 1500
#define MAX_MTU_SIZE 1500
#define PAYLOAD_SIZE(x) ((x)-IP_HDRSIZE-IP_OPTSIZE-UDP_HDRSIZE)

/*#define DEBUG*/

#endif

