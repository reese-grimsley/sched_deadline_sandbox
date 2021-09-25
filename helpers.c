#include "helpers.h"

void do_sched_setaffinity_cpu(int cpu)
{
     cpu_set_t mask;
     CPU_ZERO(&mask);
     CPU_SET(cpu, &mask); 

     if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
          perror("sched_setaffinity error. Kill me");
          // while(1);
     }
     printf("sched_getcpu = %d\n", sched_getcpu());
}

struct timespec time_diff(const struct timespec * last_time, const struct timespec * current_time)
{
     struct timespec diff;
     diff.tv_sec = current_time->tv_sec - last_time->tv_sec;
     diff.tv_nsec = current_time->tv_nsec - last_time->tv_nsec;

     if (diff.tv_nsec < 0 && diff.tv_sec == 0)
     {
          diff.tv_nsec = abs(diff.tv_nsec);
     }

     while (diff.tv_nsec < 0)
     {
          diff.tv_nsec += 1000 * 1000 * 1000;
          diff.tv_sec--;
     }
     while (diff.tv_nsec > 1000 * 1000 * 1000)
     {
          diff.tv_nsec -= 1000 * 1000 * 1000;
          diff.tv_sec++;
     }

     if (diff.tv_sec < 0)
     {
          diff.tv_sec = abs(diff.tv_sec);
          diff.tv_nsec = 1000 * 1000 * 1000 - diff.tv_nsec;

     }

     return diff;
}

void print_affinity() 
{
    cpu_set_t mask;
    long nproc, i;

    if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity error; Kill process");
        while(1);
    }
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    printf("sched_getaffinity = ");
    for (i = 0; i < nproc; i++) {
        printf("%d ", CPU_ISSET(i, &mask));
    }
    printf("\n");
}