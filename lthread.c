#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void threadf();
void readata();
int calcsum();

static __thread int a;
static int flag = 0;

// struct to hold info passed to function in new thread
struct pass_tag {
  int data;
};

typedef struct pass_tag tpass;

int
main (int argc, char *arv[])
{
  //thread ids
  pthread_t tid[2];
  
  for(int i = 0; i < 2; i++) {
    tpass *passed = (tpass *)malloc(sizeof(tpass));
    passed->data = i * 6;
    //                               id   attr      function      params
    int status = pthread_create(&(tid[i]), NULL, (void *)&threadf, passed); 
    if(status) {
      printf("thread could not be created\n");
      exit(-1);
    }
  }

  for(int i = 0; i < 3; i++) {
    //wait for nnthread to finish
    //                return value
    pthread_join(tid[i], NULL);
  }

  pthread_t t1, t2;
  pthread_create(&t1, NULL, (void *)&readata, NULL); 
  pthread_create(&t2, NULL, (void *)&calcsum, NULL);

  int result;
  pthread_join(t1, NULL);
  pthread_join(t2, (void *)&result); 

  printf("result = %d\n", result);
  
  return 0;
}

int
calcsum ()
{
  int sum = 0;

  while(flag != 1) {
    sum += 1;
    int t = 3;
    sleep(t);
  }
  return sum;
}

void
readata() {
  printf("enter character: ");
  char c = '\0';
  scanf("%c", &c);
  flag = 1;
  printf("%c\n", c);
}

void
threadf (tpass *in)
{
  
  void tcalledf();
  
  a = in->data;
  tcalledf(in);
}

void
tcalledf (tpass *args)
{
  static __thread int b = 3;
  printf("printing a = %d & b = %d from thread\n", a, b);
  free(args);
}
