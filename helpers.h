#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <assert.h>
#include <pthread.h>


struct timespec time_diff(const struct timespec * last_time, const struct timespec * current_time);
void do_sched_setaffinity_cpu(int cpu);
void print_affinity();
