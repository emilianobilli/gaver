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

#include "glo.h"

void pthreads_stop(void)
{
    pthread_mutex_init(&start_lock, NULL);
    pthread_mutex_lock(&start_lock);
}

void pthreads_start(void)
{
    pthread_mutex_unlock(&start_lock);
}

void pthread_wait_start(void)
{
    pthread_mutex_lock(&start_lock);
    pthread_mutex_unlock(&start_lock);
}
