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
#include <pthread.h>

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

struct timespec time_diff(const struct timespec * last_time, const struct timespec * current_time)
{
     struct timespec diff;
     diff.tv_sec = current_time->tv_sec - last_time->tv_sec;
     diff.tv_nsec = current_time->tv_nsec - last_time->tv_nsec;

     while (diff.tv_nsec < 0)
     {
          printf(".");
          diff.tv_nsec += 1000 * 1000 * 1000;
          diff.tv_sec--;
     }
     while (diff.tv_nsec > 1000 * 1000 * 1000)
     {
          printf(",");
          diff.tv_nsec -= 1000 * 1000 * 1000;
          diff.tv_sec++;
     }

     return diff;
}

void *run_deadline(void *data)
{
     struct sched_attr attr;
     volatile int x = 0; //no optimizing this puppy out
     int ret;
     unsigned int flags = 0;

     printf("deadline thread started [%ld]\n", gettid());

     attr.size = sizeof(attr);
     attr.sched_flags = 0;
     attr.sched_nice = 0;
     attr.sched_priority = 0;

     attr.sched_policy = SCHED_DEADLINE;
     attr.sched_runtime =  950 * 1000; //95% utilization right here
     attr.sched_period = attr.sched_deadline = 1000 * 1000;

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

     while (1) {
          x++;
          // printf("i'm doing it");
          if (x % 100000000 == 0)
          {
               printf("woah nelly, that's a lot of increments\n");
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
     printf("Start deadline_workload");
     printf("min priority: %d\t max priority: %d\r\n", sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
     
     pthread_t thread;

     printf("main thread [%ld]\n", gettid());

     pthread_create(&thread, NULL, run_deadline, NULL);

     pthread_join(thread, NULL);

     printf("main dies [%ld]\n", gettid());
     return 0;
}
