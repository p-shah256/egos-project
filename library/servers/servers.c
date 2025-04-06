/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: user friendly interfaces of system processes
 * including GPID_PROCESS, GPID_FILE and GPID_DIR
 */

#include "egos.h"
#include "servers.h"
#include <string.h>

static int sender;
static char buf[SYSCALL_MSG_LEN];

void exit(int status) {
    grass->sys_exit(status);
    while(1){grass->sys_yield();};
}

static int dir_query(int type, int dir_ino, char* name) {
    struct dir_request req;
    req.type = type;
    req.ino = dir_ino;
    strcpy(req.name, name);
    grass->sys_send(GPID_DIR, (void*)&req, sizeof(req));

    sender = GPID_DIR;
    grass->sys_recv(&sender, buf, sizeof(struct dir_reply));
    if (sender != GPID_DIR) FATAL("dir req[%d]: an error occurred", type);
    struct dir_reply *reply = (void*)buf;

    return reply->status == DIR_OK? reply->ino : -1;
}

int dir_lookup(int dir_ino, char* name) {
    return dir_query(DIR_LOOKUP, dir_ino, name);
}

int dir_mk(int dir_ino, char* name) {
    return dir_query(DIR_INSERT, dir_ino, name);
}

int dir_rm(int dir_ino, char* name) {
    return dir_query(DIR_REMOVE, dir_ino, name);
}

int file_query(int type, int file_ino, int offset, int len, char* block) {
    struct file_request req;
    req.type = type;
    req.ino = file_ino;
    req.offset = offset;
    req.len = len;
    if (type == FILE_WRITE) {
        memcpy(req.block.bytes, block, req.len);
    }
    grass->sys_send(GPID_FILE, (void*)&req, sizeof(req));

    sender = GPID_FILE;
    grass->sys_recv(&sender, buf, sizeof(struct file_reply));
    if (sender != GPID_FILE) FATAL("file req[%d]: an error occurred", type);
    struct file_reply *reply = (void*)buf;
    if (type == FILE_READ || type == FILE_GETSIZE) {
        memcpy(block, reply->block.bytes, len);
    }

    return reply->status == FILE_OK? 0 : -1;
}

int file_read(int file_ino, int offset, int len, char* block) {
    return file_query(FILE_READ, file_ino, offset, len, block);
}

int file_write(int file_ino, int offset, int len, char* block) {
    return file_query(FILE_WRITE, file_ino, offset, len, block);
}

int file_size(int file_ino) {
    m_uint32 buf[BLOCK_SIZE/sizeof(m_uint32)];
    int ret = file_query(FILE_GETSIZE, file_ino, 0, 4, (char*)buf);
    if (ret < 0) {return ret;}
    return buf[0]; // this is the file size
}
