#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#include "threadpool.h"
#include "list.h"
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

	struct work_thread * threads;
	struct list queues;
	int num_thread;
	bool shutdown;
	pthread_mutex_t lock;

	bool createdone;
	bool addeddone;
	pthread_cond_t create_cond;
	pthread_mutex_t create_mutex;

	pthread_cond_t added_cond;

};

struct work_thread {

	pthread_t id;
	struct thread_pool * pool;
	struct list queues;
};

struct future {

	fork_join_task_t task;
	void * data;
	void * results;
	pthread_cond_t workdone;
	int status;
	struct thread_pool *pool;

	struct list_elem elem;
};

static __thread bool is_worker;
static __thread struct work_thread *wthread;

static void * working_thread(void *);
static bool stop(struct thread_pool *);

/* Create a new thread pool with no more than n threads. */
struct thread_pool * thread_pool_new(int nthreads) {

	struct thread_pool *pool = malloc(sizeof(struct thread_pool));
		
	if (pool == NULL) {

		printf("Error: mallocing thread pool\n");
		exit(1);
	}
	
	pthread_mutex_init(&pool->lock, NULL);
	pthread_mutex_init(&pool->create_mutex, NULL);
	pthread_cond_init(&pool->create_cond, NULL);
	pthread_cond_init(&pool->added_cond, NULL);

	list_init(&pool->queues);

	pool->num_thread = nthreads;
	pool->shutdown = false;
	pool->createdone = false;
	pool->addeddone = false;

	pool->threads = calloc(nthreads, sizeof(struct work_thread));

	pthread_mutex_lock(&pool->create_mutex);

	for (int i = 0; i < nthreads; i++) {
		struct work_thread * current_thread = &pool->threads[i];

		list_init(&current_thread->queues);
		current_thread->pool = pool;

		pthread_create(&current_thread->id, NULL, working_thread, current_thread);
	}

	pool->createdone = true;

	wthread = malloc(sizeof(struct work_thread));
	wthread->id = pthread_self();
	list_init(&wthread->queues);
	is_worker = false;

	pthread_cond_broadcast(&pool->create_cond);
	pthread_mutex_unlock(&pool->create_mutex);

	
	return pool;
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

	pthread_mutex_lock(&pool->lock);
	// allocate a new future in this function
	struct future *return_future = malloc(sizeof(struct future));

	// initializing
	pthread_cond_init(&return_future->workdone, NULL);
	return_future->pool = pool;
	return_future->task = task;
	return_future->data = data;
	return_future->status = 0;
	
	if (is_worker) {
printf("123");
		list_push_front(&wthread->queues, &return_future->elem);
}
	else 
		list_push_back(&pool->queues, &return_future->elem);

	pool->addeddone = true;
	pthread_cond_signal(&pool->added_cond);

	pthread_mutex_unlock(&pool->lock);

	
	return return_future;
}

static void * working_thread(void * vv) {

	struct work_thread * current_thread = (struct work_thread *) vv;
	struct thread_pool * pool = current_thread->pool;

	pthread_mutex_lock(&pool->create_mutex);
	if (pool->createdone == false)
		pthread_cond_wait(&pool->create_cond, &pool->create_mutex);

	pthread_mutex_unlock(&pool->create_mutex);

	for(;;) {

		wthread = current_thread;
		is_worker = true;

		struct future * worker;

		pthread_mutex_lock(&pool->lock);

		if (pool->addeddone == false)
			pthread_cond_wait(&pool->added_cond, &pool->lock);

		while(stop(pool)) {
printf("sleep\n");
			pthread_cond_wait(&pool->added_cond, &pool->lock);
printf("sleep2\n");
		}

		if(pool->shutdown) {
printf("22\n");
			pthread_mutex_unlock(&pool->lock);
			pthread_exit(0);
		}

		if(!list_empty(&wthread->queues)) {

			struct list_elem * e = list_pop_back(&wthread->queues);
			worker = list_entry(e, struct future, elem);
		}
		else {

			if(!list_empty(&pool->queues))  {
//printf("111\n");
				struct list_elem *e = list_pop_front(&pool->queues);
				worker = list_entry(e, struct future, elem);
			}
			else {

				int i;
				struct work_thread *stealing_thread;
				for(i = 0; i < pool->num_thread; i++) {

					stealing_thread = &pool->threads[i];

					if (!list_empty(&stealing_thread->queues)) {

						break;
					}
				}

				if (i < pool->num_thread) {

					struct list_elem *e = list_pop_back(&stealing_thread->queues);
					worker = list_entry(e, struct future, elem);
				}
				else 
					worker = NULL;

			}
		}

		if (worker == NULL) {
			printf("Error: no task\n");
			break;
		}

		worker->status = 1;
		pthread_mutex_unlock(&pool->lock);

		worker->results = worker->task(pool, worker->data);

		pthread_mutex_lock(&pool->lock);
		worker->status = 2;

		pthread_cond_signal(&worker->workdone);
		pthread_mutex_unlock(&pool->lock);
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
	pthread_cond_signal(&pool->added_cond);

	pthread_mutex_unlock(&pool->lock);

	//threads join
	struct work_thread * wthread;
	for (int i = 0; i < pool->num_thread; i++) {

		wthread = &pool->threads[i];
		
		if (pthread_join(wthread->id, NULL) != 0) {

			printf("Error: thread joining\n");
			exit(1);
		}
	}

	//destroy & free
	pthread_mutex_destroy(&pool->lock);
	pthread_mutex_destroy(&pool->create_mutex);
	pthread_cond_destroy(&pool->create_cond);
	pthread_cond_destroy(&pool->added_cond);
	
	free(pool->threads);
	free(pool);
}

/* Make sure that the thread pool has completed the execution
 * of the fork join task this future represents.
 *
 * Returns the value returned by this task.
 */
void * future_get(struct future * worker) {

	pthread_mutex_lock(&worker->pool->lock);

	if (worker->status == 0) {

		worker->status = 1;

		pthread_mutex_unlock(&worker->pool->lock);

		worker->results = (worker->task)(worker->pool, worker->data);

		pthread_mutex_lock(&worker->pool->lock);

		list_remove(&worker->elem);
		worker->status = 2;
		pthread_mutex_unlock(&worker -> pool->lock);
		return worker -> results;
	}
	else {
		if (worker -> status == 2) {
			pthread_mutex_unlock(&worker -> pool -> lock);
			return worker -> results;		
		}
		if (worker->status != 2) {
			printf(" worker -> status != 2\n");		
			pthread_cond_wait(&worker->workdone, &worker->pool->lock);
		}
		pthread_mutex_unlock(&worker -> pool -> lock);	
	}
	void *result = worker->results;

	return result;
}

/* Deallocate this future.  Must be called after future_get() */
void future_free(struct future * worker) {

	if (worker == NULL) {
		printf("Error occured.");
		exit(1);
	} 
	else {

		pthread_cond_destroy(&worker->workdone);
		free(worker);
	}
}

static bool stop(struct thread_pool * pool) {

	for (int i = 0; i < pool->num_thread; i++) {

		struct work_thread * thread = &pool->threads[i];

		if(!list_empty(&thread->queues))
			return false;
	}

	return list_empty(&pool->queues) && list_empty(&wthread->queues) && !pool->shutdown;
}

