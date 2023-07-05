#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>


#define PACKET_SIZE 64

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <destination_ip>\n", argv[0]);
        exit(1);
    }
    
    char *dest_ip = argv[1];
    struct sockaddr_in dest_addr;
    
    // Create a raw socket for ICMP packets
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    
    // Set the destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    
    // Create the ICMP packet
    char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    struct icmphdr *icmp_header = (struct icmphdr *)packet;
    icmp_header->type = ICMP_ECHO;
    icmp_header->code = 0;
    icmp_header->checksum = 0;
    icmp_header->un.echo.id = getpid();
    icmp_header->un.echo.sequence = 0;
    icmp_header->checksum = htons(~(ICMP_ECHO << 8));
    
    // Send the ICMP packet
    if (sendto(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        exit(1);
    }
    
    // Close the socket
    close(sockfd);
    
    return 0;
}
