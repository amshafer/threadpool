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

#define JOBS 16

typedef struct {
  char *p_message;
  atomic_int flag;
} print_t;

void
print_func (void *arg)
{
  print_t *pt = (print_t *)arg;
  printf("%s%d\n", pt->p_message, pt->flag);
  pt->flag = 16;
  free(pt);
}

int
main (int argc, char *argv[])
{
  pool_t *p = pool_init(10);

  printf("Launching %d jobs:\n", JOBS); 
  for (int i = 0; i < JOBS; i++) {
    char message[] = "Printing from string # ";
    print_t *pt = malloc(sizeof(print_t));
    pt->p_message =  message;
    pt->flag = i;
    pool_exec(p, &print_func, pt);
  }
  printf("giving threads time to finish...\n");
  sleep(1);
  pool_destroy(p);
  
  return 0;
}
