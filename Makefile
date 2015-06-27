
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