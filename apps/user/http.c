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

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

int main(void) {
    int i;
    uip_ipaddr_t ipaddr;
    // struct timer periodic_timer, arp_timer;

    // timer_set(&periodic_timer, CLOCK_SECOND / 2);
    // timer_set(&arp_timer, CLOCK_SECOND * 10);

    uip_init();

    uip_ipaddr(ipaddr, 192, 168, 0, 2);
    uip_sethostaddr(ipaddr);
    uip_ipaddr(ipaddr, 192, 168, 0, 1);
    uip_setdraddr(ipaddr);
    uip_ipaddr(ipaddr, 255, 255, 255, 0);
    uip_setnetmask(ipaddr);

    httpd_init();
    /* Initialize the uIP TCP/IP stack */
    uip_init();

    uip_log("uIP started");

    while (1) {
        uip_len = net_recv((char *)uip_buf);
        if (uip_len > 0) {

            printf("net_recved\n");

            if (BUF->type == htons(UIP_ETHTYPE_IP)) {
                printf("IP packet received\n");
                printf("Contents:\n");
                {
                    int i;
                    for (i = 0; i < 20; i++) {
                        printf("%x ", uip_buf[i]);
                    }
                    printf("\n");
                }

                uip_arp_ipin();
                uip_input();
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if (uip_len > 0) {
                    printf("Sending IP packet\n");
                    printf("Contents:\n");
                    {
                        int i;
                        for (i = 0; i < 20; i++) {
                            printf("%x ", uip_buf[i]);
                        }
                        printf("\n");
                    }
                    uip_arp_out();
                    net_send(uip_len, (char *)uip_buf);
                }
            } else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {
                printf("ARP recieved\n");
                uip_arp_arpin();
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if (uip_len > 0) {
                    printf("sending ARP reply\n");
                    printf("contents:\n");
                    {
                        int i;
                        for (i = 0; i < 20; i++) {
                            printf("%x ", uip_buf[i]);
                        }
                        printf("\n");
                    }
                    net_send(uip_len, (char *)uip_buf);
                }
            } else {
                printf("Unknown packet type: %x\n", BUF->type);
                printf("Contents:\n");
                {
                    int i;
                    for (i = 0; i < 20; i++) {
                        printf("%x ", uip_buf[i]);
                    }
                    printf("\n");
                }
                printf("Number of bytes: %d\n", uip_len);
            }
        }
    }
    return 0;
}
