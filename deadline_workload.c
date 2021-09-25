#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

const __u64 C = 1000 * 1000 * 900;  // nsec
const __u64 T = 1000 * 1000 * 1000 * 1; 
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
     struct sched_attr attr, runtime_attr;
     volatile int x = 0; //no optimizing this puppy out
     int ret;
     unsigned int flags = 0;

     print_affinity();
     printf("deadline thread started [%ld]\n", gettid());
     printf("sched_getcpu = %d\n", sched_getcpu());

     // do_sched_setaffinity_cpu(1);

     attr.size = sizeof(attr);
     attr.sched_flags = 0;
     attr.sched_nice = 0;
     attr.sched_priority = 0;

     attr.sched_policy = SCHED_DEADLINE;
     attr.sched_runtime =  C;
     attr.sched_period = T;
     attr.sched_deadline = T;
     printf("Schedule C: [%lu]  D: [%lu]  T:[%lu]\n", attr.sched_runtime, attr.sched_deadline, attr.sched_period);

     clock_t start_cpu_time = clock();
     clock_t current_cpu_time;
     struct timespec start_wall_time, current_wall_time; 
     clock_gettime(CLOCK_REALTIME, &start_wall_time);


     ret = sched_setattr(0, &attr, flags);
     if (ret < 0) {
             done = 0;
             perror("sched_setattr");
             exit(-1);
     }
     printf("scheduling attributes set\n");
     printf("sched_getcpu = %d\n", sched_getcpu());

     // do_sched_setaffinity_cpu(1);
     // print_affinity();


     while (1) {
          x++;
          // printf("i'm doing it");
          if (x % 100000000 == 0)
          {
               sched_getattr(0, &runtime_attr, sizeof(runtime_attr), flags);
               printf("Schedule C: [%lu]  D: [%lu]  T:[%lu]\n", runtime_attr.sched_runtime, runtime_attr.sched_deadline, runtime_attr.sched_period);

               printf("woah nelly, that's a lot of increments\n");
               printf("sched_getcpu = %d\n", sched_getcpu());

               current_cpu_time = clock();
               printf("CPU time duration passed: %f s\n", (double)(current_cpu_time - start_cpu_time) / CLOCKS_PER_SEC);

               clock_gettime(CLOCK_REALTIME, &current_wall_time);
               struct timespec diff = time_diff(&start_wall_time, &current_wall_time);

               printf("Wall clock time passed: %ld s + %09ld ns\r\n\n" , diff.tv_sec, diff.tv_nsec);


          }
     }

     printf("deadline thread dies [%ld]\n", gettid());
     return NULL;
}

int main (int argc, char **argv)
{
     printf("Start deadline_workload\n");
     printf("sizeof __u64: [%d]", sizeof(__u64));
     printf("min priority: %d\t max priority: %d\r\n", sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
     
     pthread_t thread;

     printf("main thread [%ld]\n", gettid());

     pthread_create(&thread, NULL, run_deadline, NULL);

     pthread_join(thread, NULL);

     printf("main dies [%ld]\n", gettid());
     return 0;
}
