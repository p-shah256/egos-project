/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Vasu Kalariya
 * Description: a network interface
 *
 * Updated by OSI staff
 */

#include "app.h"
#include <string.h>
#include <stdlib.h>

int main() {

    SUCCESS("Enter kernel process GPID_NET");
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_NET initialization");
    grass->sys_send(GPID_PROCESS, buf, 32);

    while (1) {
        int sender, len;
        struct net_request *req = (void*)buf;
        struct net_reply *reply = (void*)buf;
        sender = 0;
        grass->sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
            case NET_SEND:
                earth->net_send(req->length, (void*)req->buf);
                reply->status = NET_OK;
                grass->sys_send(sender, (void*)reply, sizeof(*reply));
                break;
            case NET_RECV:
                reply->length = earth->net_recv(reply->buf);              
                reply->status = NET_OK;
                grass->sys_send(sender, (void*)reply, sizeof(*reply));
                break;
            default:
                FATAL("sys_net request[%d] not implemented", req->type);
        }
    }
}
