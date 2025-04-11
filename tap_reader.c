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
    int fd = tun_alloc("tap3");
    if (fd < 0) {
        fprintf(stderr, "Failed to open tap3\n");
        return 1;
    }

    printf("Listening on tap3...\n");

    unsigned char buffer[1600];
    while (1) {
        int nread = read(fd, buffer, sizeof(buffer));
        if (nread < 0) {
            perror("Reading from tap interface");
            break;
        }

        printf("Read %d bytes: ", nread);
        for (int i = 0; i < nread; ++i)
            printf("%c", buffer[i]);
        printf("\n");
    }

    close(fd);
    return 0;
}

