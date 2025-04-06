/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the disk image file (disk.img)
 * The disk image should be exactly 4MB:
 *     the first 1MB is reserved as 256 frames for memory paging;
 *     the next  1MB contains some ELF binary executables for booting;
 *     the last  2MB is managed by a file system.
 * The output is in binary format (disk.img).
 *
 * Updated by CS6640 23fall staff
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "disk.h"
#include "file.h"
#include "fs.h"

#define NKERNEL_PROC 5
char* kernel_processes[] = {
                            "../build/release/grass.elf",
                            "../build/release/sys_proc.elf",
                            "../build/release/sys_file.elf",
                            "../build/release/sys_dir.elf",
                            "../build/release/sys_shell.elf",
};

/* Inode - File/Directory mappings:
#0: /              #1: /home                #2: /home/cs6640  #3: /home/cheng
#4: /home/ta       #5: /home/cs6640/README  #6: /bin          #7: /bin/echo
#8: /bin/cat       #9: /bin/ls              #10:/bin/cd       #11:/bin/pwd
#12:/bin/clock     #13:/bin/crash1          #14:/bin/crash2   #15:/bin/ult
#16:/bin/memloop   #17:/bin/helloworld      #18:/bin/loop     #19:/bin/crash3
#20:/bin/append    #21:/bin/fstest          #23:/home/cs6640/fs (ino=100)
*/
#define NINODE 23
char* contents[] = {
                    "./   0 ../   0 home/   1   bin/   6 ",
                    "./   1 ../   0 cs6640/ 2   cheng/   3 ta/   4 ",
                    "./   2 ../   1 fs/     100 README   5 ",
                    "./   3 ../   1 ",
                    "./   4 ../   1 ",
                    "Welcome to CS6640 labs. \nThis OS is tailored from egos-2000 (https://github.com/yhzhang0128/egos-2000).\n",
                    "./   6 ../   0 echo   7 cat   8 ls   9 cd  10 pwd  11 clock  12 crash1  13 crash2  14 ult  15 memloop 16 helloworld 17 loop 18 crash3 19 append 20 fstest 21",
                    "#../build/release/echo.elf",
                    "#../build/release/cat.elf",
                    "#../build/release/ls.elf",
                    "#../build/release/cd.elf",
                    "#../build/release/pwd.elf",
                    "#../build/release/clock.elf",
                    "#../build/release/crash1.elf",
                    "#../build/release/crash2.elf",
                    "#../build/release/ult.elf",
                    "#../build/release/memloop.elf",
                    "#../build/release/helloworld.elf",
                    "#../build/release/loop.elf",
                    "#../build/release/crash3.elf",
                    "#../build/release/append.elf",
                    "#../build/release/fstest.elf",
                    "./   100 ../ 2 ", // dummy, entry to rwfs
};

char fs[FS_DISK_SIZE], exec[GRASS_EXEC_SIZE];
char rwfs[RWFS_SIZE];

void mkfs();
void mkrwfs();
inode_intf ramdisk_init();

int main() {
    mkfs();
    mkrwfs();

    /* Paging area */
    freopen("disk.img", "w", stdout);
    //write(1, exec, PAGING_DEV_SIZE); // 1MB buffer
    /* Lab5 file system */
    write(1, rwfs, RWFS_SIZE);

    /* Grass kernel processes */
    int exec_size = GRASS_EXEC_SIZE / GRASS_NEXEC;
    fprintf(stderr, "[INFO] Loading %d kernel binary files\n", NKERNEL_PROC);

    for (int i = 0; i < NKERNEL_PROC; i++) {
        struct stat st;
        stat(kernel_processes[i], &st);
        assert((st.st_size > 0) && (st.st_size <= exec_size));
        fprintf(stderr, "[INFO] Loading %s: %ld bytes\n", kernel_processes[i], (long)st.st_size);

        // open kernel process elf and use stdin (0) as file descriptor
        freopen(kernel_processes[i], "r", stdin);
        memset(exec, 0, GRASS_EXEC_SIZE);

        // read kernel process elf to the buffer "exec"
        for (int nread = 0; nread < st.st_size; )
            nread += read(0, exec + nread, exec_size - nread);

        // write kernel process elf to disk.img
        write(1, exec, st.st_size);

        // write kernel process elf again and fill in the gap
        write(1, &exec[st.st_size], exec_size - st.st_size);
    }

    // fill in empty for the remaining 1MB space
    memset(exec, 0, GRASS_EXEC_SIZE);
    write(1, exec, (GRASS_NEXEC - NKERNEL_PROC) * exec_size);

    /* File system */
    write(1, fs, FS_DISK_SIZE);
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the disk image (tools/disk.img)\n");
    return 0;
}


void mkfs() {
    inode_intf ramdisk = ramdisk_init();
    assert(treedisk_create(ramdisk, 0, NINODES) >= 0);
    inode_intf treedisk = treedisk_init(ramdisk, 0);

    char buf[GRASS_EXEC_SIZE / GRASS_NEXEC];
    for (int ino = 0; ino < NINODE; ino++) {
        if (contents[ino][0] != '#') {
            fprintf(stderr, "[INFO] Loading ino=%d, %ld bytes\n", ino, strlen(contents[ino]));
            strncpy(buf, contents[ino], BLOCK_SIZE);
            treedisk->write(treedisk, ino, 0, (void*)buf);
        } else {
            struct stat st;
            char* file_name = &contents[ino][1];
            stat(file_name, &st);

            freopen(file_name, "r", stdin);
            for (int nread = 0; nread < st.st_size; )
                nread += read(0, buf + nread, st.st_size - nread);

            fprintf(stderr, "[INFO] Loading ino=%d, %s: %d bytes\n", ino, file_name, (int)st.st_size);
            for (int b = 0; b * BLOCK_SIZE < st.st_size; b++)
                treedisk->write(treedisk, ino, b, (void*)(buf + b * BLOCK_SIZE));
        }
    }
}


int getsize() { return FS_DISK_SIZE / BLOCK_SIZE; }

int setsize() { assert(0); }

int ramread(inode_intf bs, unsigned int ino, block_no offset, block_t *block) {
    memcpy(block, fs + offset * BLOCK_SIZE, BLOCK_SIZE);
    return 0;
}

int ramwrite(inode_intf bs, unsigned int ino, block_no offset, block_t *block) {
    memcpy(fs + offset * BLOCK_SIZE, block, BLOCK_SIZE);
    return 0;
}

inode_intf ramdisk_init() {
    inode_store_t *ramdisk = malloc(sizeof(*ramdisk));

    ramdisk->read = (void*)ramread;
    ramdisk->write = (void*)ramwrite;
    ramdisk->getsize = (void*)getsize;
    ramdisk->setsize = (void*)setsize;

    return ramdisk;
}

static void mk_entry(dirent_t* entries, int i, int inum, char *name) {
    entries[i].valid = 1;
    entries[i].inum = inum;
    strcpy(entries[i].name, name);
}

void mkrwfs() {
    super_t *super = (super_t*)rwfs;
    super->magic = 0x6640;
    super->disk_size = sizeof(rwfs) / BLOCK_SIZE;

    unsigned char *map = (unsigned char*) &rwfs[BLOCK_SIZE];
    // 0: super block
    // 1: bitmap
    // 2--11: inodes
    // 12+0: root data block
    // +1: file1 data block
    // +2: dir1 data block
    // +3: file2 indirect block
    // +4-+13: file2 data block1--10
    // +14: file2 first indirect data block (12+14=26)
    // total: 27
    for (int i=0; i<27; i++) {
        map[i/8] |= (1 << (i%8));
    }

    inode_t *inodes = (inode_t*) &rwfs[INODEARR_BLOCK_START * BLOCK_SIZE];

    int root_ino = 0;
    int root_data_blk = DATA_BLOCK_START;
    int file1_ino = 1;
    int file1_data_blk = DATA_BLOCK_START + 1;
    int dir1_ino = 2;
    int dir1_data_blk = DATA_BLOCK_START + 2;
    int file2_ino = 3;
    int file2_indirect_blk = DATA_BLOCK_START + 3;
    int file2_data_blks[11] = {
        DATA_BLOCK_START + 4,
        DATA_BLOCK_START + 5,
        DATA_BLOCK_START + 6,
        DATA_BLOCK_START + 7,
        DATA_BLOCK_START + 8,
        DATA_BLOCK_START + 9,
        DATA_BLOCK_START + 10,
        DATA_BLOCK_START + 11,
        DATA_BLOCK_START + 12,
        DATA_BLOCK_START + 13,
        DATA_BLOCK_START + 14,
    };
    dirent_t *entries;
    char *data;

    // root
    inode_t *root = &inodes[root_ino];
    root->mode = MODE_D | MODE_ALL;
    root->pads[0] = 0xdeadbeef;
    root->ptrs[0] = root_data_blk;
    entries = (dirent_t*) &rwfs[BLOCK_SIZE*root_data_blk];
    mk_entry(entries, 0, root_ino, ".");
    mk_entry(entries, 1, 6640-FS_ROOT_INODE, "..");
    //   root/file1.txt
    mk_entry(entries, 2, file1_ino, "file1.txt");
    //   root/dir1
    mk_entry(entries, 3, dir1_ino, "dir1");
    root->size = 4 * sizeof(dirent_t);

    // dir1
    inode_t *dir1 = &inodes[dir1_ino];
    dir1->mode = MODE_D | MODE_ALL;
    dir1->pads[0] = 0xdeadbeef;
    dir1->ptrs[0] = dir1_data_blk;
    //   dir1/file2.txt
    entries = (dirent_t*) &rwfs[BLOCK_SIZE*dir1_data_blk];
    mk_entry(entries, 0, dir1_ino, ".");
    mk_entry(entries, 1, root_ino, "..");
    mk_entry(entries, 2, file2_ino, "file2.txt");
    dir1->size = 3 * sizeof(dirent_t);

    // file1.txt
    inode_t *file1 = &inodes[file1_ino];
    file1->mode = MODE_F | MODE_ALL;
    file1->pads[0] = 0xdeadbeef;
    file1->ptrs[0] = file1_data_blk;
    // contents
    data= (char*) &rwfs[BLOCK_SIZE*file1_data_blk];
    strcpy(data, "hello world");
    file1->size = strlen("hello world");

    // file2.txt
    inode_t *file2 = &inodes[file2_ino];
    file2->mode = MODE_F | MODE_ALL;
    file2->pads[0] = 0xdeadbeef;
    file2->indirect_ptr = file2_indirect_blk;
    for (int i=0; i<NUM_PTRS; i++) { // direct data block
        file2->ptrs[i] = file2_data_blks[i];
    }
    // indirect data blocks
    * ((m_uint32*) &rwfs[BLOCK_SIZE*file2_indirect_blk]) = file2_data_blks[10];

    // contents
    char hex[] = "0123456789abcdef,";
    int fsz = BLOCK_SIZE * 11;

    char *start = (char*) &rwfs[BLOCK_SIZE*file2_data_blks[0]];
    for (int i=0; i<fsz/17; i++) {
        memcpy(start, hex, 17);
        start += 17;
    }
    file2->size = fsz;
}
