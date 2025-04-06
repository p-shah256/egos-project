#include "fs.h"
#include "string.h"
#include "stdlib.h"

/* bit manipulation
 * see implementation in fs.c */
int bit_test(unsigned char *map, int i);

/* disk read/write
 * see implementation in fs.c */
int block_read(int block_no, void* dst);
int block_write(int block_no, void* src);

/* block allocate and free
 * see implementation in fs.c */
int alloc_block();
void free_block(int blk_id);

/* global fs_t instance */
fs_t fs;

// [lab7-ex1] initialize fs
// This function reads superblock and initialize the global fs variable "fs".
//  -- Read the definition of "fs_t" and "super_t" in "fs.h"
//  -- you should read superblock from disk by "block_read"
//  -- you should check the magic number; if not match, FATAL
//  -- you should update the "total_blk"
//  -- you should update the "avail_blks" by counting empty blocks in bitmap
//     (bit_test is a helper function to count)
void fs_init(m_uint32 super_blk_id) {
    // TODO: your code here
    FATAL("fs_init is not implemented");




}

// [lab7-ex2]
// This function reads inodes from disk.
// You should load a block from the inode arrary on disk
// which contains the inode "ino":
//   -- calculate which block contains inode "ino"
//      (note: "ino" starts from FS_ROOT_INODE)
//   -- load the block to fs.buffer_cache
//      (note: inode array starts at block INODEARR_BLOCK_START)
//   -- return the inode pointer (pointing in fs.buffer_cache)
inode_t *load_inode(int ino) {
    // TODO: your code here
    return NULL;

}

// [lab7-ex2]
// flush the contents of "fs.buffer_cache" (a list of inodes) back to disk
// notes:
//   -- you need to remember the block id of the cached inodes
//   -- later when you update files, remember to flush inodes when things change
void flush_inode() {
    // TODO: your code here

}


// [lab7-ex2]
// read "len" bytes of contents starting from "offset"
// in the file whose inode number is "ino".
// You should:
//   -- load the inode data structure
//   -- calculate which blocks (directed or indirected) contain the wanted data
//   -- copy the data to "buf"
//   -- return 0 if success; otherwise, return -1
// notes:
//   -- assume "buf" is the size of "len"
//   -- for simplicity, "len" will not exceed 1 block
int fs_read(int ino, int offset, int len, char *buf) {
    ASSERTX(len <= BLOCK_SIZE);

    // TODO: your code here
    FATAL("fs_read is not implemented");


    return 0;
}


// [lab7-ex3]
// return the size of the file "ino"
// (note: check "inode_t" in fs.h)
int fs_fsize(int ino) {
    // TODO: your code here
    FATAL("fs_fsize is not implemented");

    return 0;
}


// [lab7-ex3]
// write "len" bytes of data to the file ("ino") starting from "offset"
// notes:
//   -- use "alloc_block" to allocate a block if needed
//   -- remember to update file size
//   -- remember to flush the inode
//   -- remember to flush the modified blocks
//   -- return 0 if success; otherwise, return -1
int fs_write(int ino, int offset, int len, const char *buf) {
    // TODO: your code here
    FATAL("fs_write is not implemented");

    return 0;
}

