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
print_queue (qnl_t *q)
{
  //pthread_t cur;
  printf("Printing Queue:\n");
  /*while ((cur = qnl_dequeue(q))) {
    printf("%lu\n", (unsigned long)cur);
  }*/
}

void
pthread_test (qnl_t *q)
{
  /*
  pthread_t tid[SIZE];
  for (int i = 0; i < SIZE; i++) {
    pthread_create(&tid[i], NULL, (void *)&thread_func, NULL);
    printf("Order added to queue #%d %lu\n", i, (unsigned long)tid[i]);
    qnl_enqueue(q, tid[i]);
  }

  print_queue(q);
  
  for (int i = 0; i < SIZE; i++) {
    pthread_join(tid[i], NULL);
  } */ 
}

void
func_test (qnl_t *q)
{
  for (int i = 0; i < SIZE / 2; i++) {
    qnl_enqueue(q, qnl_exec_init(&print_queue));
    qnl_enqueue(q, qnl_exec_init(&pthread_test));
  }

  printf("print_queue = %s\n", (char *)&print_queue);
  printf("pthread_test = %s\n", (char *)&pthread_test);

  printf("Printing queue:\n");
  for (int i = 0; i < SIZE; i++) {
    printf("%s\n", (char *)qnl_dequeue(q));
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
