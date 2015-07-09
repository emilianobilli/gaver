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
#ifndef _GLO_H
#define _GLO_H

#include "types.h"
#ifdef _TABLE
#undef EXTERN
#define EXTERN
#endif

#include "heap_var.h"
#include "itc_var.h"
#include "output_var.h"
#include "input_var.h"
#include "start_var.h"

EXTERN int mtu;			/* MTU Max */
EXTERN int ifudp;		/* Socket UDP */
EXTERN int output_timer;	/* Timer for outgoing packages */
EXTERN int syn_timer;		/* Timer for syn packages */
EXTERN u_int64_t input_wait;	/* Time in nsec for waiting other message */

#endif /* _GLO_H */