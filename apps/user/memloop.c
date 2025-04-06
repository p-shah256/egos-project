#include "app.h"
#include "stdlib.h"

#define PAGSE_SIZE 4096

int main(int args, char **argv) {
    // default touching mem location
    unsigned int ptr = 0x80000000;

    // if user defines the starting mem, use it
    if (args == 2) {
        ptr = (unsigned int)strtoul(argv[1], NULL, 16);
        printf("input addr[%s] => %x\n", argv[1], ptr);
    }

    // touch by page
    ptr = (ptr / PAGE_SIZE) * PAGE_SIZE;
    printf("touch memory start: %x\n", ptr);

    int count = 0;
    while(1) {
        int i = * (int *)ptr; // touch this page
        count++;
        printf("- touched page(%d) [%x, %x)\n", count, ptr, ptr+PAGE_SIZE);
        ptr += PAGE_SIZE;
    }
}
