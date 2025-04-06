#include "app.h"
#include "string.h"
#include "stdlib.h"

void loop(int count) {
    int counter = 0;
    for(int i=0;i<count*QUANTUM; i++) {
        float ret = (float)i / counter;
        counter++;
    }
}

void sleep(int time) {
    grass->sys_sleep(time);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        loop(100);
    } else if (strcmp(argv[1], "sleep") == 0) {
        printf("Sleep for 10 QUANTUM...\n");
        sleep(10);
        printf("                   ...and waked up\n");
    }
}
