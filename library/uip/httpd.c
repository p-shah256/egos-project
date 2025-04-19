#include "egos.h"
#include "uip.h"
#include <string.h>

// HTTP "Hello World" response - the whole damn thing in one piece
static const char *hello_response = "HTTP/1.0 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Connection: close\r\n"
                                    "\r\n"
                                    "Hello World!\r\n";

#define HTTP_STATE_WAITING 0 // Waiting for request
#define HTTP_STATE_SENDING 1 // Sending response
#define HTTP_STATE_CLOSING 2 // Closing connection

void httpd_appcall(void) {
    struct http_state *s = (struct http_state *)&(uip_conn->appstate);
    if (uip_connected()) { // Handle connection state changes
        // Just connected - initialize state
        s->state = HTTP_STATE_WAITING;
        s->timer = 0;
    } else if (uip_closed() || uip_aborted() || uip_timedout()) {
        // Connection is gone, nothing to do
        return;
    }

    // Handle polling (timeout checking)
    if (uip_poll()) {
        // Increment timeout counter
        s->timer++;
        if (s->timer >= 20) { // About 20 seconds at typical poll rate
            uip_abort();
            return;
        }
    } else {
        // Reset timer on activity
        s->timer = 0;
    }

    switch (s->state) { // Process current state
    case HTTP_STATE_WAITING:
        if (uip_newdata()) { // Got request data - don't even bother parsing, just respond
            s->state = HTTP_STATE_SENDING;
            s->send_pos = 0;
            s->send_left = strlen(hello_response);
            // Fall through to sending state
        } else {
            // Still waiting for data
            break;
        }

    case HTTP_STATE_SENDING:
        if (uip_acked()) { // Last data was acknowledged, update position
            m_uint8 acked = uip_conn->len;
            if (acked > s->send_left)
                acked = s->send_left;

            s->send_pos += acked;
            s->send_left -= acked;

            if (s->send_left == 0) { // All sent, go to closing state
                s->state = HTTP_STATE_CLOSING;
                uip_close();
                return;
            }
        }

        if (uip_rexmit() || uip_newdata() || uip_acked() || uip_poll()) { // Send more data if possible
            if (s->send_left > 0) {
                uip_send(hello_response + s->send_pos, s->send_left);
            }
        }
        break;

    case HTTP_STATE_CLOSING: // Just waiting for connection to close
        break;
    }
}

void httpd_init(void) {
    // Just listen on port 80
    uip_listen(HTONS(80));
}
