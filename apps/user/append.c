/*
 * Description: a simple cmd to write to a file
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        INFO("usage: append [FILE] content");
        return -1;
    }

    /* Get the inode number of the file */
    int file_ino = dir_lookup(grass->workdir_ino, argv[1]);
    if (file_ino < 0) {
        INFO("append: file %s not found", argv[1]);
        return -1;
    }

    /* Read and print the first block of the file */
    if (strlen(argv[2]) > 0) {
        int fsz = file_size(file_ino);
        int ret = file_write(file_ino, fsz, strlen(argv[2]), argv[2]);
        if (ret != 0) {
            INFO("Error: write to %s failed", argv[1]);
            return -1;
        }
    }

    return 0;
}
