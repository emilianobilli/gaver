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
#ifndef TIMERS_H
#define TIMERS_H

#include <time.h>
#include "types.h"
#include "sock.h"
#include "mbuff.h"

struct exptimer {
    struct sock  *et_sk;	/* Sock		      */	
    struct mbuff *et_mb;	/* Memory Buffer      */

    struct timespec et_et;	/* Time to expire     */
    int    attempts;		/* Numero de intentos */
    struct exptimer *et_next;	/* Next in the queue  */
};

#ifndef _TIMERS_CODE
EXTERN void remove_et (struct sock *sk, struct mbuff *mb);
EXTERN int  register_et	(struct sock *sk, struct mbuff *mb, struct timespec *when);
EXTERN int  refresh_et	(struct exptimer *et, struct timespec *when);
EXTERN void free_et	(struct exptimer *et);
EXTERN void init_et	(void);
EXTERN struct exptimer* get_expired	(struct timespec *now);
#endif

#endif