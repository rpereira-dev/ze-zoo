#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif /* _GNU_SOURCE */
# include <sched.h>

# include <assert.h>
# include <stdio.h>
# include <unistd.h>

# include <omp.h>

int
main(void)
{
    # pragma omp parallel
    {
        int tid = omp_get_thread_num();
        unsigned int cpu, node;
        getcpu(&cpu, &node);
        usleep(tid * 1000);
        printf("Thread `%3d` running on cpu %3u of node %3u\n", tid, cpu, node);
    }

    return 0;
}

