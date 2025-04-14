#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned sum = 0;
    while (len > 1)
        sum += *buf++, len -= 2;
    if (len)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    return ~sum;
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in dest = {.sin_family = AF_INET};
    inet_pton(AF_INET, "192.168.0.2", &dest.sin_addr);

    printf("Pinging 192.168.0.2...\n");

    unsigned char packet[64] = {0};
    struct icmphdr *icmp = (struct icmphdr *)packet;
    int seq = 1;
    pid_t pid = getpid() & 0xFFFF;

    while (1) {
        memset(packet, 0, sizeof(packet));
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = pid;
        icmp->un.echo.sequence = seq++;
        icmp->checksum = 0;
        icmp->checksum = checksum(icmp, sizeof(packet));

        int sent = sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest, sizeof(dest));
        if (sent < 0) {
            perror("sendto");
            break;
        }

        printf("Sent %d bytes -> seq=%d\n", sent, icmp->un.echo.sequence);

        unsigned char buf[1024];
        struct sockaddr_in reply_addr;
        socklen_t addrlen = sizeof(reply_addr);
        int bytes = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&reply_addr, &addrlen);
        if (bytes < 0) {
            perror("recvfrom");
            continue;
        }

        struct iphdr *ip = (struct iphdr *)buf;
        struct icmphdr *rcv = (struct icmphdr *)(buf + (ip->ihl * 4));

        if (rcv->type == ICMP_ECHOREPLY && rcv->un.echo.id == pid) {
            printf("Received %d bytes <- seq=%d from %s\n\n", bytes, rcv->un.echo.sequence,
                   inet_ntoa(reply_addr.sin_addr));
        } else {
            printf("Received non-echo-reply or unmatched ID\n\n");
        }

        sleep(2);
    }

    close(sock);
    return 0;
}
