CC = gcc
CFLAGS = -lpthread 


libs: 
	$(CC) -o helpers.o helpers.c 

blocking_test: deadline_blocking_timer.c libs
	$(CC) -o dl_block.out deadline_blocking_timer.c $(CFLAGS)
load: deadline_workload.c libs
	$(CC) -o dl_load.out deadline_workload.c $(CFLAGS)

all: clean blocking_test load

clean: 
	rm dl_load.out dl_block.out