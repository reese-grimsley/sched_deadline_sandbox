CC = gcc
CFLAGS = -lpthread




blocking_test: 
	$(CC) -o dl_block.out deadline_blocking_timer.c $(CFLAGS)
load: 
	$(CC) -o dl_load.out deadline_workload.c $(CFLAGS)

all:
	blocking_test load

clean: 
	rm dl_load.out dl_block.out