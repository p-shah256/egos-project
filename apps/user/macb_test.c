/*
 * Description: a simple cmd to send a packet on ETH
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
	if (argc != 1) {
		INFO("usage: macb_test");
		return -1;
	}

	unsigned char dummy_frame[60] = {
		// Destination MAC (Broadcast)
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

		// Source MAC (your device's MAC)
		0x02, 0x00, 0x00, 0x00, 0x00, 0x01,

		// Ethertype (0x88B5 = Dummy/Experimental)
		0x88, 0xB5,

		// Payload (46 bytes of arbitrary data)
		'H', 'E', 'L', 'L', 'O', '-', 'F', 'R', 'O', 'M', 
		'-', 'M', 'A', 'C', 'B', '-', 'D', 'U', 'M', 'M', 
		'Y', '-', 'P', 'A', 'C', 'K', 'E', 'T', '-', 'T', 
		'E', 'S', 'T', '-', 'Y', 'A', 'Y', '!', ' ', 'R', 
		'/', 'W', '@', '$', 0x00, 0x00
	};
	net_send(60, (char*)(&dummy_frame));

	unsigned char buffer[100];

	int timeout = 5;
	int flag = 0;
	while(timeout--) {
		int num = net_recv((char*)(&buffer));
		if (num == 0) {
			continue;
		}
		ASSERT(num > 0, "Error while receiving data");
		flag = 1;
		if (memcmp(buffer, dummy_frame, 60)) {
			FATAL("Incorrect bytes in the frame");
		}
		break;
	}

	if (flag) {
		SUCCESS("Send recv test passed!");
	} else {
		FATAL("Failed to receive data in given time");
	}

	return 0;
}
