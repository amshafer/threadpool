/*
 * A practice program for the thread pool
 * 
 *
 * Austin Shafer - 2017
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include "queue_nl.h"
#include "threadpool.h"

#define JOBS 128000
#define THREADS 8

char message[] = "Printing from thread # ";
int kill = 0;
int glob = 0;

typedef struct {
	atomic_int flag;
} print_t;

void
print_func (void *arg)
{
	print_t *pt = (print_t *)arg;
	uint64_t tid = 0;
	printf("%s0x%lx - %d\n", message, tid, pt->flag);
	pt->flag = 16;
	free(pt);
}

void
kill_func ()
{
    /*
     * just set the global so we can end the process
     * this could probably use locks for safety, but
     * it's mostly unecessary
     */
    kill = 1;
}

int
main (int argc, char *argv[])
{
	pool_t *p = pool_init(THREADS);

	printf("Launching %d jobs:\n", JOBS); 
	for (int i = 0; i < JOBS - 1; i++) {
		print_t *pt = malloc(sizeof(print_t));
		pt->flag = i;
		pool_exec(p, &print_func, pt);
	}
	pool_exec(p, &kill_func, NULL);
	
	printf("giving threads time to finish...\n");

	while (kill == 0)
	    continue;

	pool_destroy(p);
	
	return 0;
}
