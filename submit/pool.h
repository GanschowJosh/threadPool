#ifndef pool_h
#define pool_h
#include <pthread.h>
#include "queue.h"

//the type for the worker pool, stores all the necessary information
typedef struct {
  int num_threads;
  pthread_t* workers;
  bQueue work_queue;

  pthread_mutex_t completion_mtx;
  pthread_cond_t all_done;
  int pending_jobs;
  int completed_jobs;
} worker_pool_t;

//function signatures
void wp_init(worker_pool_t*, int, int);
void wp_submit(worker_pool_t*, job_t);

void wp_wait(worker_pool_t*);

void wp_kill_workers(worker_pool_t*);
void wp_shutdown(worker_pool_t*);
void wp_cleanup(worker_pool_t*);

#endif