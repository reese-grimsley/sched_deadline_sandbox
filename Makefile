CC = gcc
CFLAGS = -lpthread -I .


libs: helpers.h
	$(CC) -c helpers.c $(CFLAGS)

blocking_test: deadline_blocking_timer.c libs
	$(CC) -o dl_block.out deadline_blocking_timer.c helpers.o $(CFLAGS)
load: deadline_workload.c libs
	$(CC) -o dl_load.out deadline_workload.c helpers.o $(CFLAGS)

all: libs blocking_test load

clean: 
	rm -f dl_load.out dl_block.out