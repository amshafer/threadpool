CC = clang
CFLAGS = -g -Wall -pthread



all: queue threadpool
	${CC} ${CFLAGS} lthread.c -o lthread

queue:
	${CC} ${CFLAGS} -fPIC -c queue_nl.c -o queue_nl.o

threadpool:
	${CC} ${CFLAGS} -fPIC -shared threadpool.c queue_nl.o -o libthreadpool.so

test: all
	${CC} ${CFLAGS} queue_test.c queue_nl.o -o queue_test
	${CC} ${CFLAGS} thread_test.c -L${PWD} -lthreadpool -o thread_test
	./thread_test

clean:
	rm -rf *~
	rm -rf *.o
	rm -rf *.so
	@rm -rf *.dSYM
	@rm -f queue_test
	@rm -f lthread
	@rm -f thread_test
