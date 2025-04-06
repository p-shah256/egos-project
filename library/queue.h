#pragma once

#include "egos.h"
#include "malloc.h"

/* Note:
 * you should test your code thoroughly.
 * It will be extremely difficult to debug your user-level threading,
 * if there is a bug in your queue implementation.
 */

typedef struct node_t {
    struct node_t *next;
    void *item;
} node_t;


typedef struct queue_t {
    struct node_t *head;
    struct node_t *tail;
} queue_t;


/* initialize queue head and tail */
void queue_init(queue_t *q) {
    q->head = (void *)0;
    q->tail = (void *)0;
}

/* add the "item" to the tail of the queue "q".
 * note: you should wrap the "item" in a "node_t"*/
void enqueue(queue_t *q, void *item) {
    /* [lab2-ex3]
     * TODO: your code here
     */


}

/* remove and return the item at the front of the queue "q".
 * note: you should free memory allocated by enqueue.
 */
void *dequeue(queue_t *q) {
    /* [lab2-ex3]
     * TODO: your code here
     */


    return (void*)0;
}


/* [lab3-ex2]
 * the following functions can be useful for your MLFQ.
 */

// check if the "target" is in the queue "q"
int in_queue(queue_t *q, void *target) {
    /* TODO: your code here */
    return 0;
}

// try to remove the "target" from the queue "q",
// but the target may or may not in the queue.
int try_rm_item(queue_t *q, void *target) {
    /* TODO: your code here */
    return 0;
}

// remove the "target" from the queue "q",
// the target must be in the queue; otherwise, there is an error
void rm_item(queue_t *q, void *target) {
    /* TODO: your code here */
}

// print the queue with whatever information you think is useful
void dump_queue(queue_t *q) {
    /* TODO: your code here */
}
