CC = gcc
CFLAGS = -lpthread -I


libs: helpers.h
	$(CC) -o helpers.o helpers.c $(CFLAGS)

blocking_test: deadline_blocking_timer.c libs
	$(CC) -o dl_block.out deadline_blocking_timer.c $(CFLAGS)
load: deadline_workload.c libs
	$(CC) -o dl_load.out deadline_workload.c $(CFLAGS)

all: clean blocking_test load

clean: 
	rm dl_load.out dl_block.out