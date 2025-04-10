#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

int tun_alloc(const char *devname) {
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;  // TAP device, no extra packet info
    strncpy(ifr.ifr_name, devname, IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    return fd;
}

int main() {
    int fd = tun_alloc("tap0");
    if (fd < 0) {
        fprintf(stderr, "Failed to open tap0\n");
        return 1;
    }

    printf("Writing on tap0...\n");
    unsigned char frame[60] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x88, 0xB5,
        'H','E','L','L','O','-','T','A','P','-','W','R','I','T','E','!',
        ' ','i','t','r',':',' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    int ctr = 1;
    while(1) {
	    if (ctr > 9) {
		    ctr = 0;
	    }
	    frame[37] = '0'+ctr;
	    usleep(1e7);
	    int nwrite = write(fd, frame, sizeof(frame));
	    if (nwrite < 0) {
		printf("Failed to write to tap interface\n");
	    } else{
		printf("wrote %d bytes, ctr: %d\n", nwrite, ctr);
	    }
	    ++ctr;
    }
    close(fd);
    return 0;
}

