#
#    This file is part of GaVer
#
#    GaVer is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    GaVer is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with GaVer.  If not, see <http://www.gnu.org/licenses/>.
#

CC := gcc
OBJ:= -c -Wall
LINK:= -lrt -lm -lpthread


all: itc.o heap.o mbuff_queue.o sockopt.o table.o


itc.o: itc.c itc.h itc_var.h
	$(CC) $(OBJ) itc.c 

heap.o: heap.c heap.h heap_var.h
	$(CC) $(OBJ) heap.c 

mbuff_queue.o: mbuff_queue.c mbuff_queue.h mbuff.h
	$(CC) $(OBJ) mbuff_queue.c

sockopt.o: sockopt.h sockopt.c
	$(CC) $(OBJ) sockopt.c

table.o: table.c glo.h
	$(CC) $(OBJ) table.c

clean:
	rm -Rf *.o