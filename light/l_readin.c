#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static int flag = 0;

void readinput();
int calcsum();

int
main (int argc, char *argv[])
{

  pthread_t t1, t2;

  pthread_create(&t1, NULL, (void *)&readinput, NULL); 
  pthread_create(&t2, NULL, (void *)&calcsum, NULL);

  int result = 0;
  pthread_join(t1, NULL);
  pthread_join(t2, (void *)&result);

  printf("result = %d\n", result);

  return 0;
}

void
readinput ()
{
  char c;
  printf("type character: ");
  c = getchar();
  flag = 1;
}

int
calcsum ()
{
  int sum = 0;
  while(!flag) {
    sum += 1;
    sleep(1);
  }
  return sum;
}
