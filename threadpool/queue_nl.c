/*
 * A lockless queue.
 *
 * Austin Shafer - 2017
 */

#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "queue_nl.h"

/*
 *  NOTE: more elegant comments in header file
 */

/*
 * A simple init function for queue nodes.
 * @param d the data to add.
 * @param n the next node.
 * @return the new node.
 */
qnode_t *
qnode_init (QDATA_T d, qnode_t *n)
{
	qnode_t *ret;
	ret = malloc(sizeof(qnode_t));
	ret->qn_data = d;
	ret->qn_next = n;

	return ret;
}

/*
 * A convenient free function for queue nodes.
 * @param out the node to destroy.
 * @return 0 on success.
 */
int
qnode_destroy (qnode_t *out)
{
	if (!out) return QERROR;

	/*
	 * The caller must free qn_data
	 */

	free(out);
	return 0;
}

qnl_t *
qnl_init ()
{
	qnl_t *ret;
	qend_t head, tail;
	
	ret = malloc(sizeof(qnl_t));

	ret->q_size = 0;

	/* initialize endpoints for atomic operations */
	head.qe_count = tail.qe_count = 0;
	head.qe_node = tail.qe_node = NULL;

	atomic_init(&ret->q_head, head);
	atomic_init(&ret->q_tail, tail);
  
	return ret;
}

int
qnl_destroy (qnl_t *q)
{
	if (!q) return QERROR;

	/*
	 * NOTE:
	 * The caller is responsible for emptying the queue and freeing
	 * all nodes, as the data in the nodes could be static.
	 */

	free(q);
	return 0;
}

/*
 * q_size is atomic so we don't use locks
 */
int
qnl_size (qnl_t *q)
{
	int ret = q->q_size;
	return ret;
}

/*
 * Enqueue a new node on the queue's tail
 * 
 */
int
qnl_enqueue (qnl_t *q, QDATA_T in)
{
	qend_t expected, new;
	
	if (!q || !in) return QERROR;
  
	qnode_t *desired = qnode_init(in, NULL);

	do {
		new = expected = atomic_load(&q->q_tail);
		new.qe_node = desired;
		new.qe_count++;
		/* set new tail */
	} while (!atomic_compare_exchange_weak(&q->q_tail, &expected, new));

	/* update old tail's next */
	if (expected.qe_node)
		expected.qe_node->qn_next = desired;
	
	/* if this is the first node, update head */
	do {
		expected = atomic_load(&q->q_head);
		new.qe_count = expected.qe_count + 1;
	} while (!expected.qe_node &&
		!atomic_compare_exchange_weak(&q->q_head, &expected, new));

	q->q_size++;
	return 0;
}

QDATA_T
qnl_dequeue (qnl_t *q)
{
	qend_t new, expected;
	QDATA_T ret = NULL;
	qnode_t *r = NULL;
	
	if (!q) return NULL;

	do {
		new = expected = atomic_load(&q->q_head);
		new.qe_node = new.qe_node->qn_next;
		new.qe_count++;
	} while (!atomic_compare_exchange_weak(&q->q_head, &expected, new));

	/* extract data and free node */
	if (expected.qe_node) {
		r = expected.qe_node;
		ret = r->qn_data;
		qnode_destroy(r);
	}

	q->q_size--;
	return ret;
}

QDATA_T
qnl_peek (qnl_t *q)
{
	qend_t head;
	
	if (!q) return NULL;

	head = atomic_load(&q->q_head);

	return head.qe_node->qn_data;
}

/*
 * THREADPOOL SPECIFIC
 *
 * Initializes a struct holding a function and its parameter struct.
 * Used to hold a queue of work to execute.
 * @param exec_f pointer to function to work with
 * @param a pointer to exec_f's argument structure
 * @return a new qnl_exec_t struct
 */
qnl_exec_t *
qnl_exec_init (void (*exec_f)(void *), void *a)
{
	qnl_exec_t *qe;
	qe = malloc(sizeof(qnl_exec_t));
	qe->qe_func = exec_f;
	qe->qe_arg = a;
	return qe;
}
