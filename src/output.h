
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


#ifndef _OUTPUT_H
#define _OUTPUT_H


#define MAX_OUTPUT_MSG 10
#define TIMER_EXPIRATION 0x01
#define ITC_EVENT	 0x02

#include "types.h"

#ifdef OUTPUT_CODE
#undef EXTERN
#define EXTERN
#endif

EXTERN void* output(void *arg);

#endif