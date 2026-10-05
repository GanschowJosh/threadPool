//file to test the bounded queue

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <math.h>
#include "queue.h"

//job arg type, used to track which jobs have been run/seen and
//tell the current job who it is (its id)
typedef struct {
  int id;

  //shared
  atomic_llong* job_sum; //sum of job ids
  atomic_int* seen; //array of counts
} job_arg_t;

//arguments for the producers
typedef struct {
  int num_to_queue;
  int first_job;
  job_arg_t* job_args;
} producer_arg_t;

//the queue in question
bQueue queue;


//function that the consumers run that logs their job in the
//shared atomic variables
void* doSomethingCool(void* arg) {
  job_arg_t* job_arg = arg;

  atomic_fetch_add(job_arg->job_sum, job_arg->id);
  atomic_fetch_add(&job_arg->seen[job_arg->id], 1);

  return NULL;
}

//function provided to the producers to enqueue N jobs
void* produce(void* arg) {
  producer_arg_t* pargs = arg;
  for(int i = 0; i < pargs->num_to_queue; ++i) {
    int id = pargs->first_job+i;
    job_t job = {.func=doSomethingCool, .arg=&pargs->job_args[id]};
    bq_enqueue(&queue, job);
  }
  pthread_exit(NULL);
}

//function provided to the consumers to keep consuming jobs until poison pill
//is encountered
void* consume(void* _) {
  while(1) {
    job_t out = bq_dequeue(&queue);
    if(out.func == NULL) break; //taking my poison pill :,(
    out.func(out.arg);
  }
  pthread_exit(NULL);
}

int main(int argc, char** argv) {
  //args:
  //  N: number of jobs to queue
  //  b: bound
  //  P: number of producers
  //  C: number of consumers
  if(argc != 5) {
    printf("run program with '{executable} {number of jobs to queue} {bound on queue} {number of producers} {number of consumers}'");
    return -1;
  }
  int N = atoi(argv[1]);
  int b = atoi(argv[2]);
  int P = atoi(argv[3]);
  int C = atoi(argv[4]);


  //initializing our queue
  bq_init(&queue, b);

  //initializing the atomic job-tracking variables
  //and lists
  atomic_llong job_sum;
  atomic_init(&job_sum, 0);

  atomic_int* seen = malloc(N*sizeof(atomic_int));
  job_arg_t* job_args = malloc(N*sizeof(job_arg_t));
  producer_arg_t* producer_args = malloc(P*sizeof(producer_arg_t));

  pthread_t producers[P];
  pthread_t consumers[C];

  for(int i = 0; i < N; ++i) {
    atomic_init(&seen[i],0);

    job_args[i].id=i;
    job_args[i].job_sum = &job_sum;
    job_args[i].seen = seen;
  }

  int startingId = 0;
  for(int i = 0; i < P; ++i) {
    int count = N/P+(i<N%P);
    producer_args[i].first_job = startingId;
    producer_args[i].num_to_queue = count;
    producer_args[i].job_args = job_args;
    startingId+=count;
    pthread_create(&producers[i], NULL, produce, &producer_args[i]);
  }

  for(int i = 0; i < C; ++i) {
    pthread_create(&consumers[i], NULL, consume, NULL);
  }

  for(int i = 0; i < P; ++i) {
    pthread_join(producers[i], NULL);
  }

  //poison pills
  for(int i = 0; i < C; ++i) {
    job_t poison = {.func = NULL, .arg = NULL};
    bq_enqueue(&queue, poison);
  }

  for(int i = 0; i < C; ++i) {
    pthread_join(consumers[i], NULL);
  }


  //results

  long long expected_job_sum = (long long)N*(N-1)/2;
  printf("Expected job sum: %lld\nActual job sum: %lld\n", expected_job_sum, (long long)atomic_load(&job_sum));

  bool all_ones = true;
  for(int i = 0; i < N; ++i) {
    //bitwise "AND-ing" everything together.
    //will result in False if any of the jobs weren't run exactly once
    all_ones &= atomic_load(&seen[i])==1;
  }
  printf("Did all jobs run exactly once? %s\n", all_ones ? "Yes" : "No");
  //=========

  //cleaning up
  bq_cleanup(&queue);
  free(producer_args);
  free(job_args);
  free(seen);
}