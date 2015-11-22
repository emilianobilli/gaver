/*
    Copyright (C) 2015 Emiliano A. Billi 
    
    This file is part of GaVer

    GaVer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GaVer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _DATAIO_H
#define _DATAIO_H

#define ATOMIC_READ 1024*1024		/* 1 MB */

#include "types.h"

#ifdef _DATAIO_CODE
#undef EXTERN
#define EXTERN
#endif

EXTERN void* dataio(void *arg);


#endif