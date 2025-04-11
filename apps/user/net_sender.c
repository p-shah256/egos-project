/*
 * Description: a simple cmd to send a packet on ETH
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 1) {
        INFO("usage: net_sender");
        return -1;
    }

	char dummy_frame[60] = {
	    // Destination MAC (Broadcast)
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	    // Source MAC (your device's MAC)
	    0x02, 0x00, 0x00, 0x00, 0x00, 0x01,

	    // Ethertype (0x88B5 = Dummy/Experimental)
	    0x88, 0xB5,

	    // Payload (46 bytes of arbitrary data)
	    'H', 'E', 'L', 'L', 'O', '-', 'F', 'R', 'O', 'M', '-', 'G', 'E', 'M', '-', 'D',
	    'U', 'M', 'M', 'Y', '-', 'P', 'A', 'C', 'K', 'E', 'T', '-', 'T', 'E', 'S', 'T',
	    '-', 'Y', 'A', 'Y', '!', ' ', 'i', 't', 'r', ':', ' ', 0x00, 0x00,
	};

	int itr = 5;
	for (int i = 0;i < 5;++i) {
		dummy_frame[58] = ('1' + i);
		net_send(60, dummy_frame);
	}

    return 0;
}
