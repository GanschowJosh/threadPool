#ifndef queue_h
#define queue_h
#include <pthread.h>

//job type, just stores a pointer to a function that returns a void*
//and accepts a void* as an argument
//also stores the argument to the job
//this way, the job can just be popped off as one and run with the arguments specified
typedef struct {
  void* (*func)(void *);
  void* arg;
} job_t;

//bounded queue type
typedef struct {
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
  int bound;
  int curr_point;
  int num_queued;
  job_t* jobs;
  pthread_mutex_t mtx;
} bQueue;

//function signatures
void bq_enqueue(bQueue*, job_t);

job_t bq_dequeue(bQueue*);

void bq_init(bQueue*, int);

void bq_cleanup(bQueue*);

#endif