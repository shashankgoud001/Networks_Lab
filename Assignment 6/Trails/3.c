#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>

#define PACKET_SIZE     4096
#define IP_HDR_SIZE     sizeof(struct ip)
#define ICMP_HDR_SIZE   sizeof(struct icmphdr)

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 ) {
        sum += *buf++;
    }
    if ( len == 1 ) {
        sum += *(unsigned char*)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char *argv[]) {
    struct hostent *host;
    struct sockaddr_in addr;
    int sockfd, count = 0, i;
    char packet[PACKET_SIZE];
    struct ip *ip_hdr = (struct ip*)packet;
    struct icmphdr *icmp_hdr = (struct icmphdr*)(packet + IP_HDR_SIZE);
    struct timeval start_time, end_time;
    long int rtt_usec;
    float rtt_msec;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    host = gethostbyname(argv[1]);
    if (!host) {
        fprintf(stderr, "Unknown host %s\n", argv[1]);
        exit(1);
    }
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    // fill ICMP header
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.id = getpid() & 0xFFFF;
    icmp_hdr->un.echo.sequence = 0;
    icmp_hdr->checksum = 0;
    icmp_hdr->checksum = checksum(icmp_hdr, ICMP_HDR_SIZE);

    // send ICMP echo request
    gettimeofday(&start_time, NULL);
    if (sendto(sockfd, packet, IP_HDR_SIZE + ICMP_HDR_SIZE, 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("sendto");
        exit(1);
    }

    // receive ICMP echo reply
    socklen_t addrlen = sizeof(addr);
    if (recvfrom(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr*)&addr, &addrlen) < 0) {
        perror("recvfrom");
        exit(1);
    }
    gettimeofday(&end_time, NULL);

    // calculate round trip time
    rtt_usec = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
    rtt_msec = (float)rtt_usec / 1000;
    printf("RTT: %.3f ms\n", rtt_msec);

    close(sockfd);
    return 0;
}
