#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <list.h>

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

	struct * work_thread threads;
	struct list queues;
	int num_thread;
	bool shutdown;
	pthread_mutex_t lock;
	pthread_cond_t wake;
}

struct work_thread {

	pthread_t id;
	struct list queues;
}

struct future {

	void * task;
	void * data;
	sem_t sem_future;
	struct thread_pool *pool;
	pthread_cond_t workdone;
}

static void * working_thread(void *);

/* Create a new thread pool with no more than n threads. */
struct thread_pool * thread_pool_new(int nthreads) {

	struct thread_pool *pool = malloc(sizeof(struct thread_pool));
		
	if (pool == NULL) {

		printf("Error: mallocing thread pool\n");
		exit(1);
	}
	
	pthread_mutex_init(&pool->lock, NULL);
	pthread_cond_init(&pool->wake, NULL);
	pool->num_thread = nthreads;
	pool->threads = calloc(nthreads, sizeof(struct work_thread));
	list_init(&pool->queues);
	pool->shutdown = false;

	pthread_mutex_lock(&pool->lock);
	
	for (int i = 0; i < nthreads; i++) {

		struct work_thread * t = &pool->threads[i];

		list_init(&t->queues);

		pthread_create(&t->id, NULL, working_thread, t);
	}

	pthread_mutex_unlock(&pool->lock);

	return pool;
}

static void * working_thread(void * vv) {

	
}

/*
 * Shutdown this thread pool in an orderly fashion.
 * Tasks that have been submitted but not executed may or
 * may not be executed.
 *
 * Deallocate the thread pool object before returning.
 */
void thread_pool_shutdown_and_destroy(struct thread_pool * pool) {

	pthread_mutex_lock(&pool->lock);

	pool->shutdown = true;

	pthread_cond_broadcast(&pool->wake);
	pthread_mutex_unlock(&pool->lock);

	//threads join
	for (int i = 0; i < pool->num_thread; i++) {

		struct work_thread * t = &pool->threads[i];
		
		if (pthread_join(t->id, NULL) != 0) {

			printf("Error: thread joining\n");
			exit(1);
		}
	}

	//destroy & free
	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->wake);
	free(pool->threads);
	free(pool);
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



