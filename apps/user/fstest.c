/*
 * Description: test cases for lab6 file system
 */

#include "app.h"
#include <string.h>

// read
void test_read() {
    int root_ino = 100;
    int file1 = dir_lookup(root_ino, "file1.txt");

    char buf[6];
    file_read(file1, 0, 5, buf);
    if (strncmp(buf, "hello world", 5) == 0) {
        SUCCESS("PASS test1");
    } else {
        INFO("FAIL test1");
    }

    file_read(file1, 2, 5, buf);
    if (strncmp(buf, "llo world", 5) == 0) {
        SUCCESS("PASS test2");
    } else {
        INFO("FAIL test2");
    }
}

int check_hex(char *buf, int shift) {
    char hex[] = "0123456789abcdef,";
    buf += 17 - (shift % 17);
    for (int i= 0; i < BLOCK_SIZE/17 -1; i++) {
        if (strncmp(buf, hex, 17) != 0) {
            printf("[%d] buf=%s\n", i, buf);
            return 0;
        }
        buf += 17;
    }

    return 1;
}

void test_read_long() {
    int root_ino = 100;
    int file2 = dir_lookup(root_ino, "dir1/file2.txt");

    char buf[BLOCK_SIZE];
    int len = BLOCK_SIZE;

    file_read(file2, 0, len, buf);
    if (check_hex(buf, 0)) {
        SUCCESS("PASS test3");
    } else {
        INFO("FAIL test3");
    }

    file_read(file2, BLOCK_SIZE*10, len, buf);
    if (check_hex(buf, (BLOCK_SIZE*10))) {
        SUCCESS("PASS test4");
    } else {
        INFO("FAIL test4");
    }

    file_read(file2, BLOCK_SIZE*10 - 3, len, buf);
    if (check_hex(buf, (BLOCK_SIZE*10 - 3))) {
        SUCCESS("PASS test5");
    } else {
        INFO("FAIL test5");
    }
}

void test_filesize() {
    int root_ino = 100;
    int file1 = dir_lookup(root_ino, "file1.txt");
    int size = file_size(file1);
    if (size == strlen("hello world")) {
        SUCCESS("PASS test6");
    } else {
        INFO("FAIL test6");
    }
}

void test_write() {
    int root_ino = 100;
    int file1 = dir_lookup(root_ino, "file1.txt");

    char buf[] = "cs6640";
    file_write(file1, 0, strlen(buf), buf);

    char buf2[11];
    file_read(file1, 0, 11, buf2);
    if (strncmp(buf2, "cs6640world", 11) == 0) {
        SUCCESS("PASS test7");
    } else {
        INFO("FAIL test7");
    }

    char buf3[12];
    file_write(file1, 6, strlen(buf), buf);
    file_read(file1, 0, 12, buf3);
    int fsz = file_size(file1);
    if (strncmp(buf3, "cs6640cs6640", 12) == 0 && fsz == 12) {
        SUCCESS("PASS test8");
    } else {
        INFO("FAIL test8");
    }
}

void test_write_long() {
    int root_ino = 100;
    int file1 = dir_lookup(root_ino, "file1.txt");

    char hex[] = "0123456789abcdef," \
                 "0123456789abcdef," \
                 "0123456789abcdef," \
                 "0123456789abcdef,";  // 17*4

    int total = 0;
    int chunk_size = strlen(hex);
    // write to max file size
    for (int i=0; i<BLOCK_SIZE*11/chunk_size; i++) {
        file_write(file1, i*chunk_size, chunk_size, hex);
        total += chunk_size;
    }

    char buf[BLOCK_SIZE];
    // check results
    file_read(file1, BLOCK_SIZE*10 - 3 - chunk_size, BLOCK_SIZE, buf);
    if (check_hex(buf, (BLOCK_SIZE*10 - 3 - chunk_size))) {
        SUCCESS("PASS test9");
    } else {
        INFO("FAIL test9");
    }

    int fsz = file_size(file1);
    if (fsz == total) {
        SUCCESS("PASS test10");
    } else {
        INFO("FAIL test10");
    }
}

int main(int argc, char** argv) {
    test_read();
    test_read_long();
    test_filesize();
    test_write();
    test_write_long();
    return 0;
}
