#ifndef SOCKET_H
#define SOCKET_H

#include "uip.h"

// States for the socket
#define SOCKET_CLOSED      0
#define SOCKET_LISTEN      1
#define SOCKET_CONNECTED   2
#define SOCKET_CLOSING     3

// Define this in your uipopt.h
typedef struct socket_state uip_tcp_appstate_t;

// Initialize the socket interface
void socket_init(void);

// Socket callback function - define this in your uipopt.h as UIP_APPCALL
void socket_appcall(void);

// Open a socket to listen on a specific port
void socket_listen(uint16_t port);

#endif /* SOCKET_H */
