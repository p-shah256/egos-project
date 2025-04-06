#include "app.h"

void ptr_is_address();
void ins_is_zero_one();

int main(int args, char **argv) {
    /* [lab1-ex3]
     * TODO: print hello world
     */

}

void ptr_is_address() {
    /* [lab1-ex5]
     * TODO: replace 0x0 with a __valid__ memory address
     */
    int *ptr = (int *)0x0;

    // read value from the ptr
    int val = *ptr;

    // write value to the ptr
    *ptr = val + 1;

    printf("ptr:%p before:%d after:%d\n", ptr, val, *ptr);
}

void ins_is_zero_one() {
    printf("Enter ins_is_zero_one()\n");

    // the instruction address where app exits
    int *exit_code = (int*) 0x820000c;

    /* [lab1-ex6]
     * TODO: replace 0x0 with the __binary__ instruction
     * of "call main".
     * How to know what is this instruction in its binary form?
     * Read "build/debug/helloworld.lst" and search "call main"
     */
    int call_main = 0x0;

    // replace with
    *exit_code = call_main;

    while(1);
}
