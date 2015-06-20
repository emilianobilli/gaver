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
#include "mbuff.h"

/*
 * ToDo: mbuff_insert()
 */

void mbuff_enqueue (struct mb_queue *queue, struct mbuff *mb);
struct mbuff *mbuff_dequeue(struct mb_queue *queue);

void pktbuff_enqueue (struct pkt_queue *queue, struct pktbuff *pkt);
struct pktbuff *pktbuff_dequeue(struct pkt_queue *queue);


