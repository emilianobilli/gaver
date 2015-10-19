/*
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
    along with GaVer.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _COMMON_H
#define _COMMON_H

#include <sys/types.h>

#define IOMSG_ACTION_READ	0x00
#define IOMSG_ACTION_WRITE	0x01

#ifndef _COMMON_CODE
extern ssize_t write_msg(int sd, void *msg, size_t size);
extern ssize_t read_msg(int sd, void *msg, size_t size);
#endif

#endif