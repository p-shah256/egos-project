#include "uip.h"
#include <string.h>

// Simple HTTP "Hello World" response
static const char *hello_response = "HTTP/1.0 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Connection: close\r\n"
                                    "\r\n"
                                    "Hello World!\r\n";

#define STATE_WAITING 0
#define STATE_OUTPUT  1

static PT_THREAD(handle_output(struct httpd_state *s)) {
    PT_BEGIN(&s->outputpt);

    // Send our complete hello_response directly
    PSOCK_BEGIN(&s->sout);
    PSOCK_SEND_STR(&s->sout, hello_response);
    PSOCK_CLOSE(&s->sout);
    PSOCK_END(&s->sout);

    PT_END(&s->outputpt);
}

static PT_THREAD(handle_input(struct httpd_state *s)) {
    PSOCK_BEGIN(&s->sin);

    // We just need to consume the incoming HTTP request
    // No need to parse it since we'll respond with Hello World regardless
    PSOCK_READTO(&s->sin, '\n');
    // Set state to output so we respond with Hello World
    s->state = STATE_OUTPUT;
    while (1) { // Skip the rest of the HTTP headers
        PSOCK_READTO(&s->sin, '\n');
        if (s->inputbuf[0] == '\r' || s->inputbuf[0] == '\n') {
            break;
        }
    }
    PSOCK_END(&s->sin);
}

static void handle_connection(struct httpd_state *s) {
    handle_input(s);
    if (s->state == STATE_OUTPUT) {
        handle_output(s);
    }
}

void httpd_appcall(void) {
    struct httpd_state *s = (struct httpd_state *)&(uip_conn->appstate);

    if (uip_closed() || uip_aborted() || uip_timedout()) {
        // Connection closed, nothing to do
    } else if (uip_connected()) {
        // New connection
        PSOCK_INIT(&s->sin, s->inputbuf, sizeof(s->inputbuf) - 1);
        PSOCK_INIT(&s->sout, s->inputbuf, sizeof(s->inputbuf) - 1);
        PT_INIT(&s->outputpt);
        s->state = STATE_WAITING;
        s->timer = 0;
        handle_connection(s);
    } else if (s != NULL) {
        // Existing connection
        if (uip_poll()) {
            ++s->timer;
            if (s->timer >= 20) {
                uip_abort(); // Timeout after 20 polls
            }
        } else {
            s->timer = 0;
        }
        handle_connection(s);
    } else {
        uip_abort(); // Invalid state
    }
}

void httpd_init(void) {
    uip_listen(HTONS(80)); // Listen on port 80
}
