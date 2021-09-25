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

#include "helpers.h"

#define gettid() syscall(__NR_gettid)

#define SCHED_DEADLINE       6

/* XXX use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr           314
#define __NR_sched_getattr           315
#endif

#ifdef __i386__
#define __NR_sched_setattr           351
#define __NR_sched_getattr           352
#endif

#ifdef __arm__
#define __NR_sched_setattr           380
#define __NR_sched_getattr           381
#endif

const struct timespec SLEEP_DURATION = {.tv_sec = 5, .tv_nsec = 0};
const uint64_t C = 1000 * 1000 * 10;  // nsec
const uint64_t T = 1000 * 1000 * 1000 * 2; 
static volatile int done;

struct sched_attr {
     __u32 size;

     __u32 sched_policy;
     __u64 sched_flags;

     /* SCHED_NORMAL, SCHED_BATCH */
     __s32 sched_nice;

     /* SCHED_FIFO, SCHED_RR */
     __u32 sched_priority;

     /* SCHED_DEADLINE (nsec) */
     __u64 sched_runtime;
     __u64 sched_deadline;
     __u64 sched_period;
};

int sched_setattr(pid_t pid,
               const struct sched_attr *attr,
               unsigned int flags)
{
     return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
               struct sched_attr *attr,
               unsigned int size,
               unsigned int flags)
{
     return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

void *run_deadline(void *data)
{
     struct sched_attr attr;
     int x = 0;
     int ret;
     unsigned int flags = 0;

     printf("deadline thread started [%ld]\n", gettid());
     printf("sched_getcpu = %d\n", sched_getcpu());


     attr.size = sizeof(attr);
     attr.sched_flags = 0;
     attr.sched_nice = 0;
     attr.sched_priority = 0;

     attr.sched_policy = SCHED_DEADLINE;
     attr.sched_runtime =  C;
     attr.sched_period = T;
     attr.sched_deadline = T;
     printf("Schedule C: [%lu]  D: [%lu]  T:[%lu]\n", attr.sched_runtime, attr.sched_deadline, attr.sched_period);

     ret = sched_setattr(0, &attr, flags);
     if (ret < 0) {
             done = 0;
             perror("sched_setattr");
             exit(-1);
     }
     // do_sched_setaffinity_cpu(1);
     printf("scheduling attributes set\n");
     printf("sched_getcpu = %d\n", sched_getcpu());

     struct timespec current_time, remaining_time, last_time, sleep_duration;
     memcpy(&sleep_duration, &SLEEP_DURATION , sizeof(struct timespec));
     printf("We shall sleep for %ld s + %09ld ns\r\n" , sleep_duration.tv_sec, sleep_duration.tv_nsec);


     while (!done) {
          int return_code = nanosleep(&sleep_duration, &remaining_time);
          if (return_code != 0) {
               printf("return code indicates we did not sleep the full duration. Code %d", return_code);

          }

          memcpy(&last_time, &current_time, sizeof(struct timespec));
          clock_gettime(CLOCK_REALTIME, &current_time);
          printf("Current time is %ld s + %09ld ns\r\n" , current_time.tv_sec, current_time.tv_nsec);
          struct timespec diff = time_diff(&last_time, &current_time);
          printf("Time difference since last iteration is %ld s + %09ld ns\r\n" , diff.tv_sec, diff.tv_nsec);
          diff.tv_sec -= sleep_duration.tv_sec;
          diff.tv_nsec -= sleep_duration.tv_nsec;
          printf("Delay correctness: %ld s + %09ld ns\r\n" , diff.tv_sec, diff.tv_nsec);

          printf("sched_getcpu = %d\n", sched_getcpu());

          x++;

     }

     printf("deadline thread dies [%ld]\n", gettid());
     return NULL;
}

int main (int argc, char **argv)
{
     printf("Start deadline with blocking (timer)");
     printf("min priority: %d\t max priority: %d\r\n", sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
     
     pthread_t thread;

     printf("main thread [%ld]\n", gettid());

     pthread_create(&thread, NULL, run_deadline, NULL);

     sleep(30);

     done = 1;
     pthread_join(thread, NULL);

     printf("main dies [%ld]\n", gettid());
     return 0;
}
