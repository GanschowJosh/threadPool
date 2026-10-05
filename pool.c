#include <pthread.h>
#include <stdlib.h>

#include "pool.h"
#include "queue.h"

//this is the main function for each of the workers
void* worker_main(void* arg) {
  //worker is provided with a pointer to the pool it belongs to
  worker_pool_t* pool = arg;

  //loop until we get a poison job
  while(1) {
    job_t job = bq_dequeue(&pool->work_queue);

    //poison job
    if(job.func == NULL) break;

    //run the job
    job.func(job.arg);


    //grab mutex and track that it completed the job
    pthread_mutex_lock(&pool->completion_mtx);

    pool->pending_jobs--;
    pool->completed_jobs++;

    //if we have no jobs left, signal that we completed all
    if(pool->pending_jobs == 0)
      pthread_cond_signal(&pool->all_done);

    pthread_mutex_unlock(&pool->completion_mtx);
  }
  return NULL;
}


//initializer function for the worker pool
void wp_init(worker_pool_t* pool, int num_threads, int bound) {

  //initializes all the attributes of the worker pool
  pool->num_threads = num_threads;
  pool->workers = malloc(num_threads*sizeof(pthread_t));
  pthread_cond_init(&pool->all_done, NULL);
  pthread_mutex_init(&pool->completion_mtx, NULL);
  bq_init(&pool->work_queue, bound);
  pool->pending_jobs = 0;
  pool->completed_jobs = 0;

  //creates the worker threads and starts them
  for(int i = 0; i < num_threads; ++i) {
    pthread_create(&pool->workers[i], NULL, worker_main, pool);
  }
}

//submit a job to the queue
void wp_submit(worker_pool_t* pool, job_t job) {
  pthread_mutex_lock(&pool->completion_mtx);
  pool->pending_jobs++;
  pthread_mutex_unlock(&pool->completion_mtx);
  bq_enqueue(&pool->work_queue, job);
}

//wait until all submitted jobs are completed
void wp_wait(worker_pool_t* pool) {
  pthread_mutex_lock(&pool->completion_mtx);
  while(pool->pending_jobs > 0)
    pthread_cond_wait(&pool->all_done, &pool->completion_mtx);
  pthread_mutex_unlock(&pool->completion_mtx);
}

//cleanup function to queue the poison jobs
void wp_kill_workers(worker_pool_t* pool) {
  for(int i = 0; i < pool->num_threads; ++i) {
    job_t poison = {.func=NULL, .arg=NULL};
    //directly use the queue instead of using the submit button,
    //these jobs should not count toward the completed/pending counters
    bq_enqueue(&pool->work_queue, poison);
  }
}

//full shutdown function, waits for all poison jobs to complete
void wp_shutdown(worker_pool_t* pool) {
  wp_kill_workers(pool);
  for(int i = 0; i < pool->num_threads; ++i) {
    pthread_join(pool->workers[i], NULL);
  }
}

//cleans up heap variables and pthread items
void wp_cleanup(worker_pool_t* pool) {
  free(pool->workers);
  bq_cleanup(&pool->work_queue);
  pthread_cond_destroy(&pool->all_done);
  pthread_mutex_destroy(&pool->completion_mtx);
}