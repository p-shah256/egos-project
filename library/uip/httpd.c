#include "egos.h" // Include necessary headers for m_uint types if not standard
#include "uip.h"
#include <string.h>

static const char *hello_response = "HTTP/1.0 200 OK\r\n"
                                    "Content-Type: text/html\r\n"
                                    "Connection: close\r\n"
                                    "\r\n"
                                    "<h1> EgOS is ALIVE! First contact established! </h1>\n"
                                    "<p> This is a simple HTTP server response. </p> \n"
                                    " ███████  ██████   ██████  ███████ \n"
                                    " ██      ██       ██    ██ ██      \n"
                                    " █████   ██   ███ ██    ██ ███████ \n"
                                    " ██      ██    ██ ██    ██      ██ \n"
                                    " ███████  ██████   ██████  ███████ \n";

#define HTTP_STATE_WAITING 0
#define HTTP_STATE_SENDING 1
#define HTTP_STATE_CLOSING 2

// UIP will add the overhead of the TCP/IP stack, so we need to ensure
// we don't exceed that
#define MAX_DRIVER_SEND_BYTES 256 - 100

void httpd_appcall(void) {
    struct http_state *s = (struct http_state *)&(uip_conn->appstate);

    if (uip_connected()) {
        // Initialize state for a new connection
        s->state = HTTP_STATE_WAITING;
        s->pos = 0;
        s->last_sent_len = 0;
        s->data_ptr = NULL;
        s->total_len = 0;
        return; // Nothing more to do now, wait for request
    }

    if (uip_closed() || uip_aborted() || uip_timedout()) {
        s->state = HTTP_STATE_CLOSING;
        return;
    }

    if (uip_acked()) {
        // Data we sent previously was acknowledged by the remote host.
        if (s->last_sent_len > 0) {
            s->pos += s->last_sent_len; // Advance our position marker
            s->last_sent_len = 0;       // Reset the 'last sent' counter
        }
    }

    // Handle Incoming Data (Client Request)
    if (uip_newdata() && s->state == HTTP_STATE_WAITING) {
        // We received data (presumably the HTTP request) and were waiting for it.
        // In this simple example, we ignore the request content and just
        // prepare to send our fixed response.
        s->state = HTTP_STATE_SENDING;
        s->data_ptr = hello_response;
        s->total_len = strlen(hello_response);
        s->pos = 0;           // Start sending from the beginning
        s->last_sent_len = 0; // Nothing has been sent yet for this response
    }

    // Handle Sending Data, in the following cases:
    // 1. We just received a request (uip_newdata was true).
    // 2. Previous data was ACKed (uip_acked is true), maybe freeing up window.
    if (s->state == HTTP_STATE_SENDING &&
        (uip_newdata() || uip_acked() || uip_poll() || uip_rexmit())) {

        // Check if there is data left to send
        if (s->pos < s->total_len) {
            m_uint16 remaining_len = s->total_len - s->pos;
            m_uint16 mss = uip_mss(); // TCP Layer's Max Segment Size

            m_uint16 max_allowed_send = mss;
            if (max_allowed_send > MAX_DRIVER_SEND_BYTES) {
                max_allowed_send = MAX_DRIVER_SEND_BYTES;
            }

            // Now calculate the actual chunk length for *this* transmission
            m_uint16 chunk_len = remaining_len;
            if (chunk_len > max_allowed_send) {
                chunk_len = max_allowed_send;
            }

            if (chunk_len > 0) {
                uip_send(s->data_ptr + s->pos, chunk_len);
                // Record how much we *attempted* to send in this call.
                // We only update s->pos when this data is ACKed (in uip_acked).
                s->last_sent_len = chunk_len;
                // IMPORTANT: After calling uip_send, we typically MUST return.
                // uIP will process the sent data and call us again later.
                return;
            }
        } else {
            // We should close the connection, but ONLY after the very last chunk is acked.
            if (s->last_sent_len == 0) {
                s->state = HTTP_STATE_CLOSING;
                uip_close();
            }
            // Otherwise, we are just waiting for the final ACK. Do nothing now.
            return;
        }
    }
}

void httpd_init(void) {
    uip_listen(HTONS(80));
    uip_log("HTTP server listening on port 80\n");
}
