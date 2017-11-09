#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#include "threadpool.h"
#include "threadpool_lib.h"

/**
 * threadpool.h
 *
 * A work-stealing, fork-join thread pool.
*/
/*
 * Opaque forward declarations. The actual definitions of these
 * types will be local to your threadpool.c implementation.
 */
struct thread_pool {

	int num_thread;
	bool check;
	pthread_mutex_t poolMutex;
}


struct future {

	// semaphore
	sem_t sem_future;
	// pool, task, data
	struct thread_pool *pool;
	void * task;
	void * data;
	pthread_mutex_t future_mutex;
}


/* Create a new thread pool with no more than n threads. */
struct thread_pool * thread_pool_new(int nthreads) {

	struct thread_pool *pool = malloc(sizeof(struct thread_pool));
		
	if (pool == NULL) {

		exit(1);
	}	
	
	pool->num_thread = nthreads;
}

/*
 * Shutdown this thread pool in an orderly fashion.
 * Tasks that have been submitted but not executed may or
 * may not be executed.
 *
 * Deallocate the thread pool object before returning.
 */
void thread_pool_shutdown_and_destroy(		struct thread_pool *) {

}
/* A function pointer representing a ’fork/join’ task.
 * Tasks are represented as a function pointer to a
 * function.
 * ’pool’ - the thread pool instance in which this task
            executes
 * ’data’ - a pointer to the data provided in thread_pool_submit
 *
 * Returns the result of its computation.
 */
typedef void * (* fork_join_task_t) (struct thread_pool *pool, void * data) {

}

/*
 * Submit a fork join task to the thread pool and return a
 * future.  The returned future can be used in future_get()
 * to obtain the result.
 * ’pool’ - the pool to which to submit
 * ’task’ - the task to be submitted.
 * ’data’ - data to be passed to the task’s function
 *
 * Returns a future representing this computation.
 */
struct future * thread_pool_submit(struct thread_pool *pool, fork_join_task_t task, void * data) {
	// allocate a new future in this function
	struct future future = malloc(sizeof(struct future));

	// initializing
	sem_init(&return_future->sem_future,0,0);
	future->pool = pool;
	future->task = task;
	future->data = data;
}
/* Make sure that the thread pool has completed the execution
 * of the fork join task this future represents.
 *
 * Returns the value returned by this task.
 */
void * future_get(struct future *) {

}

/* Deallocate this future.  Must be called after future_get() */
void future_free(struct future * future_thread) {
	if (future_thread == NULL) {
		printf("Error occured.");
	} else {
		// Think of the two cases; when "future_thread is not finished and when it finished"
		// Destroy it semaphore and mutex;
		// -> order whould be 1. semaphore and mutex
		// This means I need to initialize semaphore and mutex in future struct
		sem_destroy(future_thread->sem_future);
		pthread_mutex_destroy(future_thread->future_mutex);
		free(future_thread);
	}
	

}



