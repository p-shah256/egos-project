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
    int fd = tun_alloc("tap2");
    if (fd < 0) {
        fprintf(stderr, "Failed to open tap2\n");
        return 1;
    }

    printf("Reading on tap2...\n");
    unsigned char frame[1024];

    int ctr = 0;
    int nread = 0;
    while(1) {
        nread = read(fd, frame, sizeof(frame));
        if (nread < 0) {
            printf("Error reading from tap interface");
            exit(0);
        }
        if (nread == 60) {
            for (int i = 0;i < nread;++i) {
                printf("%c", frame[i]);
            }
            printf("\n");
            for (int i = 0;i < nread;++i) {
                printf("%x ", frame[i]);
            }
            printf("\n");
            break;
        }
    }

    usleep(1e6);
    printf("Writing to tap2\n");
    int nwrite = write(fd, frame, nread);
    if (nwrite < 0) {
        printf("Error writing from tap interface");
        exit(0);
    }
    usleep(1e5);

    close(fd);
    return 0;
}

