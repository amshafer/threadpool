/*
 * A quick queue test program
 *
 * Austin Shafer 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "queue_nl.h"

#define SIZE 8

void
thread_func () {
  printf("I am thread %lu\n", (unsigned long)pthread_self());
}

void
print_queue (void *i)
{
  printf("Printing entry: %p\n", i);
}


void
func_test (qnl_t *q)
{
  for (int i = 0; i < SIZE; i++) {
    qnl_enqueue(q, qnl_exec_init(&print_queue, i));
  }

  printf("Printing all queue contents:\n");
  for (int i = 0; i < SIZE; i++) {
    qnl_exec_t *e = qnl_dequeue(q);
    e->qe_func(e->qe_arg);
  }
}

int
main (int argc, char *argv[])
{
  qnl_t *q = qnl_init();

  // pthread_test(q);
  func_test(q);
  
  qnl_destroy(q);
  return 0;
}
