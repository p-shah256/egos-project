#include "fs.h"
#include "string.h"
#include "stdlib.h"

extern fs_t fs; // defined in fs_rw.c
inode_t *load_inode(int ino);
void flush_inode();

/* bitmap functions
 */
void bit_set(unsigned char *map, int i)
{
    map[i/8] |= (1 << (i%8));
}
void bit_clear(unsigned char *map, int i)
{
    map[i/8] &= ~(1 << (i%8));
}
int bit_test(unsigned char *map, int i)
{
    return map[i/8] & (1 << (i%8));
}

/* disk read/write */
int block_read(int block_no, char* dst) {
    return earth->disk_read(block_no, 1, dst);
}

int block_write(int block_no, char* src) {
    return earth->disk_write(block_no, 1, src);
}

/* block allocate and free */
int alloc_block() {
    int ret;
    for (int i=0; i<fs.total_blks; i++) {
        if (!bit_test(fs.bitmap, i)) {
            bit_set(fs.bitmap, i);
            return i;
        }
    }
    FATAL("fs.c: alloc_block runs out of blocks");
    return -1;
}

void free_block(int blk_id) {
    bit_clear(fs.bitmap, blk_id);
    // clean block
    char buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);
    block_write(blk_id, buf);
}


// ASSUMPTION: one data-block
static int lookup_name(inode_t *inode, char *token) {
    int inum;
    dirent_t dirent_arr[NUM_DIRENT_BLOCK];
    earth->disk_read(inode->ptrs[0], 1, (char*)&dirent_arr);
    int name_exists = 0;
    for (int i = 0; i < NUM_DIRENT_BLOCK; i++) {
        if (dirent_arr[i].valid == 1 &&
            strcmp(dirent_arr[i].name, token) == 0) {
            inum = dirent_arr[i].inum;
            name_exists = 1;
            break;
        }
    }
    if (!name_exists) {
        return -1;
    }
    return inum + FS_ROOT_INODE;
}


int fs_dir_lookup(int dir_ino, const char *path) {
    char *_path = strdup(path);

    int inum = dir_ino;
    int found = 0;
    char *token = strtok(_path, "/");
    while (token != NULL) {
        inode_t *inode = load_inode(inum);
        if (inode->mode & MODE_D) {
            // inum is the next file
            inum = lookup_name(inode, token);
            if (inum < 0) {return inum;}
        } else { // should not see regular file
            free(_path);
            return -1;
        }

        token = strtok(NULL, "/");
    }
    free(_path);

    return inum;
}
