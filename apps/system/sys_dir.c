/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the directory system server
 * handling requests to directory lookup, insertion, etc.
 */

#include "app.h"
#include <string.h>
#include <stdlib.h>
#include "fs.h"

/* This is querying the initial readonly fs*/
int dir_do_lookup(int dir_ino, char* name) {
    char buf[BLOCK_SIZE];
    file_read(dir_ino, 0, BLOCK_SIZE, buf);

    for (int i = 0, namelen = strlen(name); i < strlen(buf) - namelen; i++)
        if (!strncmp(name, buf + i, namelen) &&
            buf[i + namelen] == ' ' && (i == 0 || buf[i - 1] == ' '))
            return atoi(buf + i + namelen);

    return -1;
}

int main() {
    SUCCESS("Enter kernel process GPID_DIR");

    /* Send a notification to GPID_PROCESS */
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_DIR initialization");
    grass->sys_send(GPID_PROCESS, buf, 31);

    /* Wait for directory requests */
    while (1) {
        int sender;
        struct dir_request *req = (void*)buf;
        struct dir_reply *reply = (void*)buf;
        sender = 0;
        grass->sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case DIR_LOOKUP:
            if (req->ino < FS_ROOT_INODE) {
                reply->ino = dir_do_lookup(req->ino, req->name);
            } else {
#ifdef RWFSON
                int ret = fs_dir_lookup(req->ino, req->name);
                // this is a hack:
                // in rwfs, inode=6640 points to the parent of the mount point
                reply->ino = (ret == 6640) ? 2 /*/home/cs6640*/ : ret;
#else
                FATAL("rwfs is off; should not be here");
#endif
            }
            reply->status = reply->ino == -1? DIR_ERROR : DIR_OK;
            grass->sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case DIR_INSERT:
        case DIR_REMOVE:
        default:
            FATAL("sys_dir: request%d not implemented", req->type);
        }
    }
}
