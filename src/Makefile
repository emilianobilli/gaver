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
OBJ:= -c -Wall -ggdb
OBJ_LIB:= -c -Wall -fPIC
LINK:= -lrt -lm -lpthread
SHARED:= -shared 
USER:=$(shell whoami)

all: itc.o heap.o mbuff_queue.o sockopt.o table.o output.o util.o input.o dataio.o start.o gvd.o configk.o sock.o common.o kernel_api.o kernel.o timers.o dump.o kernel_util.o kernel_tx.o libgv.so
	$(CC) itc.o heap.o mbuff_queue.o sockopt.o table.o output.o util.o input.o dataio.o start.o configk.o sock.o gvd.o common.o kernel_api.o kernel.o timers.o dump.o kernel_util.o kernel_tx.o -o gvd $(LINK)

libgv.so: libgv.o gv_err.o common.o
	$(CC) $(SHARED) libgv.o common.o gv_err.o -o libgv.so

libgv.o: libgv.c
	$(CC) $(OBJ_LIB) libgv.c

gv_err.o: gv_err.h gv_err.c
	$(CC) $(OBJ_LIB) gv_err.c

itc.o: itc.c itc.h itc_var.h
	$(CC) $(OBJ) itc.c 

heap.o: heap.c heap.h heap_var.h
	$(CC) $(OBJ) heap.c 

kernel_api.o: kernel_api.c kernel_api.h
	$(CC) $(OBJ) kernel_api.c

kernel.o: kernel.c kernel.h
	$(CC) $(OBJ) kernel.c

kernel_tx.o: kernel_tx.c kernel_tx.h
	$(CC) $(OBJ) kernel_tx.c

mbuff_queue.o: mbuff_queue.c mbuff_queue.h mbuff.h
	$(CC) $(OBJ) mbuff_queue.c

sockopt.o: sockopt.h sockopt.c
	$(CC) $(OBJ) sockopt.c

table.o: table.c glo.h
	$(CC) $(OBJ) table.c

output.o: output.c 
	$(CC) $(OBJ) output.c

util.o: util.c util.h
	$(CC) $(OBJ) util.c

input.o: input.c input.h
	$(CC) $(OBJ) input.c

dataio.o: dataio.c dataio.h
	$(CC) $(OBJ) dataio.c

configk.o: configk.c configk.h
	$(CC) $(OBJ) configk.c

timers.o: timers.h timers.c
	$(CC) $(OBJ) timers.c

start.o: start.c start_var.h start.h
	$(CC) $(OBJ) start.c
common.o: common.c
	$(CC) $(OBJ_LIB) common.c

sock.o: sock.c
	$(CC) $(OBJ) sock.c

kernel_util.o: kernel_util.c kernel_util.h
	$(CC) $(OBJ) kernel_util.c

dump.o: dump.c
	$(CC) $(OBJ) dump.c

gvd.o: gvd.c
	$(CC) $(OBJ) gvd.c

main.o:
	$(CC) $(OBJ) main.c


install: all
	-sudo mkdir /opt/gaver
	-sudo chown $(USER) /opt/gaver
	-mkdir /opt/gaver/bin
	-mkdir /opt/gaver/etc
	-mkdir /opt/gaver/dev
	-mkdir /opt/gaver/run
	cp gvd /opt/gaver/bin
	cp ../script/config-parser.sh /opt/gaver/bin
	cp ../script/gaverd.sh /opt/gaver/bin
	cp ../script/gvd.ini /opt/gaver/etc
	-sudo mkdir /usr/include/gaver/
	sudo cp apitypes.h /usr/include/gaver/
	sudo cp gv_err.h   /usr/include/gaver/
	sudo cp libgv.h    /usr/include/gaver/
	sudo cp libgv.so   /usr/lib/

clean:
	-rm -Rf *.o
	-rm main
	-rm libgv.so
	-rm gvd

