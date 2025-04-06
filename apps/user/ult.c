/*
 * (C) 2023, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse
 * Description: queue and user-level threading
 * Students implement a queue and a threading package;
 * And then spawn multiple threads as either producer or consumer.
 *
 * Updated by CS6640 23fall staff
 */

#include "app.h"
#include "queue.h"

/** These two functions are defined in grass/context.S **/
void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);

/** Multi-threading functions **/
void thread_create();
void thread_yield();
void thread_exit();


struct thread {
    /* [lab2-ex1]
     * TODO: your code here
     */

};


void thread_init(){
    /* [lab2-ex4]
     * TODO: your code here
     */

    INFO("thread_init is not implemented\n");
    exit(0);
}


void thread_create(void (*f)(void *), void *arg, unsigned int stack_size) {
    /* [lab2-ex4]
     * TODO: your code here
     * note: stack grows from HIGH to LOW in memory
     */

    INFO("thread_create is not implemented\n");
    exit(0);
}



void ctx_entry(void){
    /* [lab2-ex4]
     * TODO: your code here
     */

    INFO("ctx_entry is not implemented\n");
    exit(0);
}

void thread_yield(){
    /* [lab2-ex5]
     * TODO: your code here
     */

    INFO("thread_yield is not implemented\n");
    exit(0);
}

void thread_exit(){
    /* [lab2-ex5]
     * TODO: your code here
     */

    INFO("thread_exit is not implemented\n");
    exit(0);
}

/* Main and test cases */
void test_create();
void test_stack();
void test_exit();
void test_single_yield();
void test_pingpong_yield();
void test_producer_consumer();

int main() {
    INFO("User-level threading is not implemented.");
    while(1);

    /* test cases for [lab2-ex4]
     * These tests touch thread_init, thread_create, and ctx_entry.
     * You should run one test at a time.
     */
    // test_create();
    // test_stack();

    /* test case for [lab2-ex5]
     * These tests touch all functions.
     * You should run one test at a time.
     */
    // test_exit();
    // test_single_yield();
    // test_pingpong_yield();
    // test_producer_consumer();

    return 0;
}

// ------- test cases ----------

/* Testing thread function */

static void hello(void *arg){
    if (arg == NULL) {
        printf("Hello without arg\n");
    } else {
        printf("Hello with %s\n", arg);
    }
}

static void stack(void *arg){
    static int i = 128;
    while(i>0) {
        --i;
        printf("stak: nested call %d\n", i);
        stack(NULL);
    }

    if (arg != NULL) {
        printf("stack is %s\n", arg);
    }
}

static void many_yield(void *arg) {
    int i = 0;
    for (; i<102400; i++) {
        thread_yield();
    }
    printf("pass: many yield[%s]: i=%d\n", arg, i);
}

/** Producer and consumer functions **/

#define NSLOTS	3

static char *slots[NSLOTS];
static unsigned int in, out;

static void producer(void *arg){
    for (;;) {
        // first make sure there's an empty slot.
        // then add an entry to the slots
        // lastly, let other threads run

        while(slots[in] != (void*)0) {
            thread_yield();
        }

        slots[in++] = arg;
        if (in == NSLOTS) in = 0;
        thread_yield();
    }
}

static void consumer(void *arg){
    for (int i = 0; i < 5; i++) {
        // first make sure there's something in the buffer
        // then grab an entry to the slot
        // lastly, let other threads run

        while(slots[out] == (void*)0) {
            thread_yield();
        }

        void *x = slots[out];
        slots[out] == (void*)0;
        if (++out == NSLOTS) out = 0;
        printf("%s: got '%s'\n", arg, x);
        thread_yield();
    }
}

/* if you pass this, you should see
 *  "Hello with world"
 */
void test_create() {
    thread_init();
    thread_create(hello, "world", 1024);
    thread_exit();
}

/* if you pass this, you should see
 * """
 * stak: nested call 127
 * ...
 * stak: nested call 0
 * stack is fine
 * """
 */
void test_stack() {
    thread_init();
    thread_create(stack, "fine", 16*1024);
    thread_exit();
}

/* if you pass this, you should see:
 * """
 * Hello without arg
 * Hello with world 1
 * Hello with world 2
 * Hello with main
 * """
 */
void test_exit() {
    thread_init();
    thread_create(hello, NULL, 1024);
    thread_create(hello, "world 1", 1024);
    thread_create(hello, "world 2", 1024);
    hello("main");
    thread_exit();
}

/* if you pass this, you should see:
 *  "pass: many yield[single]: i=102400"
 */
void test_single_yield() {
    thread_init();
    thread_create(many_yield, "single", 1024);
    thread_exit();
}

/* if you pass this, you should see:
 * """
 * pass: many yield[ping]: i=102400
 * pass: many yield[pong]: i=102400
 * """
 */
void test_pingpong_yield() {
    thread_init();
    thread_create(many_yield, "ping", 1024);
    thread_create(many_yield, "pong", 1024);
    thread_exit();
}

/* if you pass this, you should see:
 * """
 * consumer 5: got 'producer 1'
 * consumer 5: got 'producer 2'
 * consumer 4: got 'producer 1'
 * consumer 3: got 'producer 1'
 * consumer 2: got 'producer 2'
 * consumer 1: got 'producer 1'
 * consumer 5: got 'producer 1'
 * consumer 4: got 'producer 2'
 * consumer 3: got 'producer 1'
 * consumer 2: got 'producer 1'
 * consumer 1: got 'producer 2'
 * consumer 5: got 'producer 1'
 * consumer 4: got 'producer 1'
 * consumer 3: got 'producer 2'
 * consumer 2: got 'producer 1'
 * consumer 1: got 'producer 1'
 * consumer 5: got 'producer 2'
 * consumer 4: got 'producer 1'
 * consumer 3: got 'producer 1'
 * consumer 2: got 'producer 2'
 * consumer 1: got 'producer 1'
 * consumer 4: got 'producer 1'
 * consumer 3: got 'producer 2'
 * consumer 2: got 'producer 1'
 * consumer 1: got 'producer 1'
 * <then, running forever>
 * """
 */
void test_producer_consumer() {
    thread_init();

    thread_create(consumer, "consumer 1", 1024);
    thread_create(consumer, "consumer 2", 1024);
    thread_create(consumer, "consumer 3", 1024);
    thread_create(consumer, "consumer 4", 1024);
    thread_create(consumer, "consumer 5", 1024);
    thread_create(producer, "producer 1", 1024);
    producer("producer 2");
    thread_exit();
}
