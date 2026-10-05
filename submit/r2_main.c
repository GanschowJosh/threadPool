#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <time.h>
#include <unistd.h>

#include "queue.h"
#include "pool.h"

//helper from https://stackoverflow.com/a/53708448
//finds the delta between two timespecs
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    int NS_PER_SECOND = 1000000000;
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

//simple workload to give us some computation time
void* doSomethingCool(void* _) {
  int boing = 1;
  for(int i = 0; i < 100000000; ++i) {
    boing += (boing == 10 ? -10 : 1);
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("use program with `{executable} {number of workers} {number of jobs to queue}`");
    return 3;
  }

  int N = atoi(argv[1]);
  int M = atoi(argv[2]);

  //our pool to test
  worker_pool_t pool;

  wp_init(&pool, N, 32);

  //start timer
  struct timespec start;
  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  //just queue up M jobs, no arguments needed
  for(int i = 0; i < M; ++i) {
    job_t job_to_queue = {.func=doSomethingCool, .arg=NULL};
    wp_submit(&pool, job_to_queue);
  }

  wp_wait(&pool);

  //stop timer
  clock_gettime(CLOCK_MONOTONIC, &end);

  wp_shutdown(&pool);

  wp_cleanup(&pool);

  struct timespec diff;
  sub_timespec(start, end, &diff);
  double time_taken = (double)diff.tv_sec + (double)diff.tv_nsec/1000000000;

  printf("Number of worker threads: %d\nNumber of jobs processed: %d\nTotal elapsed time: %f\nJobs processed per second: %f\n", N, pool.completed_jobs, time_taken, (double)M/time_taken);
}