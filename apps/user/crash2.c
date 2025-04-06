/* Description: run supervisor-mode instructions
 * A user-level app should be killed by the kernel.
 * Test both before and after the first time being scheduled.
 * */

#include "app.h"

int main(int argc, char** argv) {
    if (argc != 1) {
        grass->sys_yield();
    }
    asm("sfence.vma zero,zero");
    SUCCESS("Crash2 succeeds in running high-privileged instructions");
}

