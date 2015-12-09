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
#ifndef _KERNEL_DEFS_H
#define _KERNEL_DEFS_H

#include "types.h"

#define DATA_IO_NONE		0x00	/* 0000 */
#define DATA_IO_WAITING_MEM	0x04	/* 0100 */
#define DATA_IO_READ_PENDING	0x01	/* 0001 */
#define DATA_IO_WRITE_PENDING	0x02	/* 0010 */
#define DATA_IO_RW_PENDING	0x03	/* 0011 = IO_READ_PENDING | IO_WRITE_PENDING */

#endif