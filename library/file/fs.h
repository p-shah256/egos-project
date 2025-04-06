#pragma once
#include "egos.h"
#include "disk.h"

/* how many buckets of size M do you need to hold N items? */
#define DIV_ROUND_UP(N, M) ((N) + (M) - 1) / (M)
/* number of direct pointers */
#define NUM_PTRS 10
/* number of indirect pointers */
#define NUM_INPTRS (BLOCK_SIZE / sizeof(m_uint32))
/* file size */
#define FILE_SIZE (BLOCK_SIZE * (NUM_PTRS + NUM_INPTRS))
/* number of directory entries (dirent_t) in one block */
#define NUM_DIRENT_BLOCK (BLOCK_SIZE / sizeof(struct fs_dirent))

//#define SUPER_BLOCK_ID       0  /* disk.h */
#define BITMAP_BLOCK_ID      1
#define INODEARR_BLOCK_START 2
#define DATA_BLOCK_START     12
#define DATA_BLOCK_END       2047

/* Superblock */
struct fs_super {
    m_uint32 magic;       /*==0x6640*/
    m_uint32 disk_size;   /* in blocks */

    /* pad out to an entire block */
    char pad[BLOCK_SIZE - 2 * sizeof(m_uint32)];
};

struct fs_inode {
    m_uint32 mode;
    m_uint32 size;     /* file size in bytes */
    m_uint32 pads[3];
    m_uint32 ptrs[NUM_PTRS]; /* direct data block pointer */
    m_uint32 indirect_ptr; /* indirect pointer */
};

/* Entry in a directory */
struct fs_dirent {
    m_uint32 valid : 1;
    m_uint32 inum  : 31;
    char name[28];         /* with trailing NUL */
};


/* in-memory fs data structure*/
struct fs_struct {
    int super_blk_id;
    int total_blks;
    int avail_blks;

    // page caches
    unsigned char bitmap[BLOCK_SIZE];
    int buffer_blk_id;
    unsigned char buffer_cache[BLOCK_SIZE];
};

typedef struct fs_super super_t;
typedef struct fs_inode inode_t;
typedef struct fs_dirent dirent_t;
typedef struct fs_struct fs_t;

/*
 file or dir
 |<-   ->|                   |<- S-app ->|<- U-app ->|
 +---+---+---+---+---+---+---+---+---+---+---+---+---+
 | F | D |   |   |   |   |   | R | W | X | R | W | X |
 +---+---+---+---+---+---+---+---+---+---+---+---+---+
*/

#define MODE_F  (1 << 31)
#define MODE_D  (1 << 30)
#define MODE_UW (1 << 1)
#define MODE_UR (1 << 2)
#define MODE_SW (1 << 4)
#define MODE_SR (1 << 5)
#define MODE_UALL (MODE_UW | MODE_UR)
#define MODE_SALL (MODE_SW | MODE_SR)
#define MODE_ALL  (MODE_UALL | MODE_SALL)

// must be aligned with mount point in rofs (see mkfs.c)
#define FS_ROOT_INODE 100

/* fs functions */
int belong_rwfs(char* path);

void fs_init(m_uint32 super_blk_id);
int  fs_dir_lookup(int dir_ino, const char *path);
int  fs_read(int ino, int offset, int len, char *buf);
int  fs_write(int ino, int offset, int len, const char *buf);
int  fs_fsize(int ino);

//int  fs_create(int dir_ino, const char *name);
//int  fs_remove(int dir_ino, const char *name);
