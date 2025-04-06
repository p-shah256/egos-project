/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the file (inode) system server
 * handling requests to reading and writing inodes
 *
 * updated by CS6640 staff
 */

#include "app.h"
#include "file.h"
#include <string.h>
#include "fs.h"




int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    /* Initialize the file system interface */
    inode_intf fs = treedisk_init(fs_disk_init(), 0);
#ifdef RWFSON
    fs_init(SUPER_BLOCK_ID);
#endif

    /* Send a notification to GPID_PROCESS */
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_FILE initialization");
    grass->sys_send(GPID_PROCESS, buf, 32);

    /* Wait for inode read/write requests */
    while (1) {
        int sender, r;
        struct file_request *req = (void*)buf;
        struct file_reply *reply = (void*)buf;
        sender = 0;
        grass->sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case FILE_READ:
            if (req->ino < FS_ROOT_INODE) {
                r = fs->read(fs, req->ino, req->offset, (void*)&reply->block); // always read a block
                reply->status = r == 0 ? FILE_OK : FILE_ERROR;
            } else {
#ifdef RWFSON
                r = fs_read(req->ino, req->offset, req->len, (void*)&reply->block);
                reply->status = r == 0 ? FILE_OK : FILE_ERROR;
#else
                FATAL("rwfs is off; should not be here");
#endif
            }
            grass->sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case FILE_WRITE:
            if (req->ino < FS_ROOT_INODE) {
                INFO("sys_file: rofs does not support write (ino=%d)", req->ino);
                reply->status = FILE_ERROR;
            } else {
#ifdef RWFSON
                r = fs_write(req->ino, req->offset, req->len, (void*)&req->block);
#else
                FATAL("rwfs is off; should not be here");
#endif
                reply->status = r == 0 ? FILE_OK : FILE_ERROR;
            }
            grass->sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case FILE_GETSIZE:
            if (req->ino < FS_ROOT_INODE) {
                INFO("sys_file: rofs does not support file_size (ino=%d)", req->ino);
                reply->status = FILE_ERROR;
            } else {
#ifdef RWFSON
                int fsz = fs_fsize(req->ino);
                reply->status = (fsz >= 0) ? FILE_OK : FILE_ERROR;
                *(m_uint32*)reply->block.bytes = fsz; // store the size in the first 4B
#else
                FATAL("rwfs is off; should not be here");
#endif
            }
            grass->sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        default:
            FATAL("sys_file: request[%d] not implemented", req->type);
        }
    }
}
