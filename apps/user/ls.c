/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple ls
 *
 * Updated by CS6640 staff 23fall
 */

#include "app.h"
#include <string.h>
#include "fs.h"

int main(int argc, char** argv) {
    if (argc > 1) {
        INFO("ls: ls with args not implemented");
        return -1;
    }

    /* Read the directory content */
    char buf[BLOCK_SIZE];
    file_read(grass->workdir_ino, 0, BLOCK_SIZE, buf);

    if (grass->workdir_ino < FS_ROOT_INODE) {
        /* Remove the inode numbers from the string */
        for (int i = 1; i < strlen(buf); i++)
            if (buf[i - 1] == ' ' && buf[i] >= '0' && buf[i] <= '9') buf[i] = ' ';

        /* Print the directory content */
        printf("%s\r\n", buf);
    } else {
#ifdef RWFSON
        /* this is fs-specific */
        dirent_t* entries = (dirent_t*) buf;
        for (int i=0; i<NUM_DIRENT_BLOCK; i++) {
            if (entries[i].valid) {
                printf("%s   ", entries[i].name);
            }
        }
        printf("\n");
#else
        FATAL("rwfs is off; should never be here");
#endif
    }

    return 0;
}
