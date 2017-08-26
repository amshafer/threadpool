/*
 * A personal practice program for pthreads
 * 
 * A simple queue.
 *
 * Austin Shafer - 2017
 */
#ifndef _QNL_H_
#define _QNL_H_

// A macro to compile this into whatever type we want
#define QDATA_T qnl_exec_t *
// custom free function to destroy QDATA_T objects
#define Q_FREE(arg) free(arg)

/*
 * struct to hold function pointers to return
 *
 */
typedef struct {
  void (*qe_func)(void *);
  void *qe_arg;
} qnl_exec_t;

qnl_exec_t *qnl_exec_init();
// free func_t
void qnl_exec_destroy (qnl_exec_t *qe);

/*
 * A node in a linked list queue.
 * Singly linked for simplicity.
 *
 */
struct _qnode_t {
  QDATA_T qn_data;                  // the data in this node
  struct _qnode_t *qn_next;         // the next node in the chain
};

// for ease of writing
typedef struct _qnode_t qnode_t;


/*
 * The queue struture. This is what the user holds a reference to.
 * Holds size, head and tail of list, and a lock to access the queue.
 * Use should look like:
 *       qnl_t *queue = qnl_init();
 *       qnl_enqueue(queue, data_pointer);
 *       qnl_dqueue(queue);
 *       qnl_destroy(queue);
 */
struct queue_t {
  atomic_int qa_size;                // The size of the queue
  qnode_t *q_next;                  // the head of the list
  qnode_t *q_tail;                  // the end of the list
  // NOTE: maybe one day I will be smart enough to make this atomic ;)
  pthread_mutex_t *q_lock;          // Lock to add/remove from queue
};

typedef struct queue_t qnl_t;

/*
 * Initialization function for a queue.
 * see queue_t struct comment for details.
 * @return queue structure
 */
qnl_t *qnl_init();

/*
 * Destruction function for a queue.
 * Frees data in nodes. (provide custom free in macro above)
 * @param q the queue to be destroyed.
 * @return 0 for success
 */
int qnl_destroy(qnl_t *q);

/*
 * Adds an element to the end of the queue.
 * @param q the queue to add to.
 * @param in the data to add as a new element.
 * @return 0 for success
 */
int qnl_enqueue(qnl_t *q, QDATA_T in);

/*
 * returns the element at the head of the queue.
 * @param q the queue to retrieve from.
 * @return the element's data.
 */
QDATA_T qnl_dequeue(qnl_t *q);

/*
 * returns the data in the element at the head of the queue
 * but DOES NOT dequeue the element.
 * @param q the queue to peek. 
 * @return the element's data.
 */
QDATA_T qnl_peek(qnl_t *q);

#endif
