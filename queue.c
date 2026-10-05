#include "queue.h"
#include <stdlib.h>

//enqueue a job to the bounded queue
void bq_enqueue(bQueue* queue, job_t job_to_add) {
  //grab the mutex
  pthread_mutex_lock(&queue->mtx);
  //wait until the queue is not_full (prevents enqueueing more than the bound)
  while(queue->num_queued == queue->bound)
    pthread_cond_wait(&queue->not_full, &queue->mtx);

  //uses a couple variables:
  //current point is the "left" of the queue, the next to be popped off
  //num queued is just the number of jobs queued
  //using these, we can decide where the "right" end of the queue is and append to it
  queue->jobs[(queue->curr_point+queue->num_queued)%queue->bound]=job_to_add;
  queue->num_queued++;
  pthread_cond_signal(&queue->not_empty);
  pthread_mutex_unlock(&queue->mtx);
}

//dequeues a job and returns it
job_t bq_dequeue(bQueue* queue) {
  pthread_mutex_lock(&queue->mtx);
  while(queue->num_queued == 0)
    pthread_cond_wait(&queue->not_empty, &queue->mtx);
  job_t popped = queue->jobs[queue->curr_point];
  //tracking our current point: just add one and mod by the bound to keep it in bounds
  //also subtract one from the number queued
  queue->curr_point = (queue->curr_point+1)%queue->bound;
  queue->num_queued--;
  pthread_cond_signal(&queue->not_full);
  pthread_mutex_unlock(&queue->mtx);
  return popped;
}

//initialize the bounded queue
void bq_init(bQueue* queue, int bound) {
  queue->bound = bound;
  queue->curr_point = 0;
  queue->num_queued = 0;
  queue->jobs = malloc(bound*sizeof(job_t));

  pthread_mutex_init(&queue->mtx, NULL);
  pthread_cond_init(&queue->not_full, NULL);
  pthread_cond_init(&queue->not_empty, NULL);
}

//cleanup the bounded queue, freeing heap variables and pthread items
void bq_cleanup(bQueue* queue) {
  free(queue->jobs);
  pthread_cond_destroy(&queue->not_full);
  pthread_cond_destroy(&queue->not_empty);
  pthread_mutex_destroy(&queue->mtx);
}