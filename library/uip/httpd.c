#include "egos.h"
#include "uip.h"
#include <string.h>

static const char *hello_response = "HTTP/1.0 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Connection: close\r\n"
                                    "\r\n"
                                    "*** EgOS is ALIVE! First contact established! ***\n";

#define HTTP_STATE_WAITING 0 // Waiting for request
#define HTTP_STATE_SENDING 1 // Sending response
#define HTTP_STATE_CLOSING 2 // Closing connection

void httpd_appcall(void) {
    struct http_state *s = (struct http_state *)&(uip_conn->appstate);

    // New connection
    if (uip_connected()) {
        s->state = 0;
        s->pos = 0;
    }

    // Data received - send response right away
    if (uip_newdata()) {
        s->state = 1;
        // Send the entire response in one go
        uip_send(hello_response, strlen(hello_response));
    }

    // Data acknowledged - close when done
    if (uip_acked()) {
        uip_close();
    }
}

void httpd_init(void) {
    uip_listen(HTONS(80));
    uip_log("HTTP server listening on port 80\n");
}
