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
#ifndef _ITC_VAR_H
#define _ITC_VAR_H

#include "itc.h"

EXTERN struct itc_msg_queue kernel_netout_queue[3];	/* Messages FROM Kernel TO Network Output */	
EXTERN struct itc_msg_queue netinp_kernel_queue;	/* Messages FROM Network Input to Kernel */
EXTERN struct itc_msg_queue netout_kernel_queue;	/* Messages FROM Network Output to Kernel */
EXTERN struct itc_msg_queue dataio_kernel_queue;	/* Messages FROM Data IO to Kernel */
EXTERN struct itc_msg_queue kernel_dataio_queue;	/* Messages FROM Kernel to Data IO */
EXTERN pthread_t thread_table[THREAD_TABLE_SZ];

EXTERN int itc_event;	/* Signal ITC event file descriptor */

#endif /* itc_var.h */
