/**
 * \addtogroup helloworld
 * @{
 */

/**
 * \file
 *         An example of how to write uIP applications
 *         with protosockets.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/*
 * This is a short example of how to write uIP applications using
 * protosockets.
 */

/*
 * We define the application state (struct hello_world_state) in the
 * hello-world.h file, so we need to include it here. We also include
 * uip.h (since this cannot be included in hello-world.h) and
 * <string.h>, since we use the memcpy() function in the code.
 */
#include "app.h"
#include "servers.h"
#include <string.h>
// #include "timer.h"

int main(void) {
    // TODO: get timer working
    // struct timer periodic_timer, arp_timer;
    int i;

    /* Initialize the uIP TCP/IP stack */
    uip_init();

    /* Initialize our application */
    hello_world_init();
    uip_log("uIP started");

    /* Main processing loop */
    while (1) {
        /* Check for incoming packets */
        uip_len = net_recv((char *)uip_buf);

        if (uip_len > 0) {
            printf("Received %u bytes\n", uip_len);
            for (i = 0; i < uip_len; ++i) {
                printf("%02x ", ((unsigned char *)uip_buf)[i]);
            }

            uip_arp_ipin();
            uip_input();
            /* If the above function invocation resulted in data that
               should be sent out on the network, the global variable
               uip_len is set to a value > 0. */
            if (uip_len > 0) {
                uip_arp_out();
                net_send(uip_len, (char *)uip_buf);
            }
        }
    }

    return 0;
}
