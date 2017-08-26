CC = clang
CFLAGS = -g -Wall -pthread



all: queue threadpool
	${CC} ${CFLAGS} lthread.c -o lthread

queue:
	${CC} ${CFLAGS} -c queue_nl.c -o queue_nl.o

threadpool:
	${CC} ${CFLAGS} -fpic -shared threadpool.c queue_nl.o -o libthreadpool.so

test: all
	${CC} ${CFLAGS} queue_test.c queue_nl.o -o queue_test
	${CC} ${CFLAGS} thread_test.c -L$(shell pwd) -lthreadpool -o thread_test
	./thread_test

clean:
	rm -rf *~
	rm -rf *.o
	rm -rf *.so
