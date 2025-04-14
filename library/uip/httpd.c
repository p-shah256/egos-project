#include "httpd.h"
#include "uip.h"
#include <string.h>

static const char *hello_response = "HTTP/1.0 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Connection: close\r\n"
                                    "\r\n"
                                    "Hello World!\r\n";

void httpd_appcall(void) {
    if (uip_connected()) {
        // When connection established, just send our response
        uip_send(hello_response, strlen(hello_response));
    } else if (uip_acked()) {
        // When data acknowledged, close the connection
        uip_close();
    }
}

void httpd_init(void) {
    // Listen on port 80
    uip_listen(HTONS(80));
}
