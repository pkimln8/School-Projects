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

	bool createdone;
	pthread_cond_t create_cond;
	pthread_mutex_t create_mutex;

	pthread_cond_t wake;
}

struct work_thread {

	pthread_t id;
	struct thread_pool * pool;
	struct list queues;
}

struct future {

	void * task;
	void * data;
	void * results;
	sem_t workdone;
	int status;
	struct thread_pool *pool;
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
	list_init(&pool->queues);	
	pool->num_thread = nthreads;
	pool->threads = calloc(nthreads, sizeof(struct work_thread));
	pool->shutdown = false;
	pool->createdone = false;
	
	for (int i = 0; i < nthreads; i++) {

		struct work_thread * current_thread = &pool->threads[i];

		list_init(&current_thread->queues);
		current_thread->pool = pool;

		pthread_create(&t->id, NULL, working_thread, current_thread);
	}

	pool->createdone =true;
	pthread_cond_broadcast(&pool->create_cond);
	return pool;
}

static void * working_thread(void * vv) {

	struct work_thread * current_thread = (struct work_thread *) vv;
	struct thread_pool * pool = current_thread->pool;

	pthread_mutex_lock(&pool->create_mutex);
	if (pool->createdone == false);
		pthead_cond_wait(&pool->create_cond);

	pthread_mutex_unlock(&pool->create_mutex);

	for(;;) {

		struct future * worker;

		pthread_mutex_lock(&pool->lock);

		if(!list_empty(&current_thread->queues)) {

			struct list_elem * e = list_pop_back(&current_thread->queues);
			worker = list_entry(e, struct future, elem);
		}
		else {

			if(!list_empty(&pool->queues)) {

				struct list_elem *e = list_pop_front(&pool->queues);
				worker = list_entry(e, struct future, elem);
			}
			else {

				int i;
				struct work_thread *stealing_thread;
				for(i = 0; i < pool->num_thread; i++) {

					stealing_thread = pool->threads[i];

					if (!list_empty(stealing_thread->queues))
						break;
				}

				if (i < pool->num_thread) {

					struct list_elem *e = list_pop_front(&stealing_thread->queues);
					worker = list_entry(e, struct future, elem);
				}
				else {

					pthread_mutex_unlock(&pool->lock);

					if (pool->shutdown)
						pthread_exit(0);

					
				}
			}
		}

		pthread_mutex_unlock(&pool->lock);
		
		worker->status = 1;
		worker->results = worker->task(pool, worker->data);
		worker->status = 2;

		sem_post(&worker->workdone);

	}
	return NULL;
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
	struct future *return_future = malloc(sizeof(struct future));

	// initializing
	sem_init(&return_future->workdone, 0, 0);
	return_future->pool = pool;
	return_future->task = task;
	return_future->data = data;
	
	pthread_mutex_lock(&pool->lock);
	list_push_back(&pool->queues, &return_future->elem);
	return_future->status = 0;
	pthread_mutex_unlock(&pool->lock);

	
	return return_future;
}
/* Make sure that the thread pool has completed the execution
 * of the fork join task this future represents.
 *
 * Returns the value returned by this task.
 */
void * future_get(struct future * worker) {

	pthread_mutex_lock(&worker->pool->lock);

	if (worker->status == 0) {

		list_remove(&worker->elem);
		worker->status = 1;

		pthread_mutex_unlock(&worker->pool->lock);

		worker->results = (worker->task)(worker->pool, worker->data);

		pthread_mutex_lock(&worker->pool->lock);

		worker->status = 2;
	}
	else {

		if (worker->status != 2)
			sem_wait(&worker->workdone);
	}

	void *result = worker->results;
	pthread_mutex_unlock(&worker->pool->lock);

	return result;
}

/* Deallocate this future.  Must be called after future_get() */
void future_free(struct future * worker) {

	if (worker == NULL) {
		printf("Error occured.");
		exit(1);
	} 
	else {
		// Think of the two cases; when "future_thread is not finished and when it finished"
		// Destroy it semaphore and mutex;
		// -> order whould be 1. semaphore and mutex
		// This means I need to initialize semaphore and mutex in future struct
		if (worker->status == 2) {

			sem_destroy(&worker->sem_future);
			free(worker);
		}
		else {

			sem_wait(&worker->workdone);
			sem_destroy(&worker->workdone);
			free(worker);
		}
	}
}

