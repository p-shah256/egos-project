#include "socket.h"
#include "uip.h"
#include "uipopt.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
void socket_init(void) {
    // Nothing specific needed for initialization
}

/*---------------------------------------------------------------------------*/
void socket_listen(uint16_t port) {
    // Start listening on the specified port
    uip_listen(HTONS(port));
}

/*---------------------------------------------------------------------------*/
void socket_appcall(void) {
    struct socket_state *s = (struct socket_state *)&(uip_conn->appstate);

    if (uip_connected()) {
        // Initialize socket state when connection is established
        s->state = SOCKET_CONNECTED;
        s->port = uip_conn->lport;
        s->buffer_len = 0;
        printf("Socket connected on port %d\n", HTONS(s->port));

    } else if (uip_closed() || uip_aborted() || uip_timedout()) {
        // Connection closed
        if (s->state != SOCKET_CLOSED) {
            printf("Socket closed on port %d\n", HTONS(s->port));
            s->state = SOCKET_CLOSED;
        }

    } else if (s->state == SOCKET_CONNECTED && uip_newdata()) {
        // New data received
        printf("Received packet on port %d, length: %d\n", HTONS(s->port), uip_datalen());
        printf("From IP: %d.%d.%d.%d, port: %d\n", uip_conn->ripaddr[0], uip_conn->ripaddr[1],
               uip_conn->ripaddr[2], uip_conn->ripaddr[3], HTONS(uip_conn->rport));

        // Print packet contents (as hex and ASCII)
        printf("Packet contents:\n");
        for (uint16_t i = 0; i < uip_datalen(); i++) {
            printf("%02x ", ((uint8_t *)uip_appdata)[i]);
            if ((i + 1) % 16 == 0)
                printf("\n");
        }
        printf("\n");

        // For simple echo functionality, can uncomment this
        // memcpy(uip_appdata, uip_appdata, uip_datalen());
        // uip_send(uip_appdata, uip_datalen());
    }
}
