/*
 * Description: a simple cmd to receive a packet on ETH
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 1) {
        INFO("usage: net_sender");
        return -1;
    }

    char frame[256];
    while (1) {
	int len = net_recv((char*)frame);
	if (len <= 0) {
	    continue;
	}
	INFO("Received %d bytes", len);
	for (int i = 0;i < len;++i) {
	    printf("%c", frame[i]);
	}
	printf("\n");
    }

    return 0;
}
