#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <poll.h>
#include <netinet/ip_icmp.h>
#include <time.h>

#define PACKET_SIZE 1024
#define MAX_HOPS 30

unsigned short checksum(unsigned short *buf, int len)
{
    unsigned int sum = 0;
    while (len > 1)
    {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1)
    {
        sum += *(unsigned char *)buf;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con)
{
    struct hostent *host_entity;
    char *ip = (char *)malloc(NI_MAXHOST * sizeof(char));
    int sockfd;

    if ((host_entity = gethostbyname(addr_host)) == NULL)
    {
        return NULL;
    }

    strcpy(ip, inet_ntoa(*(struct in_addr *)host_entity->h_addr));

    addr_con->sin_family = AF_INET;
    addr_con->sin_port = htons(0);
    addr_con->sin_addr.s_addr = inet_addr(ip);

    return ip;
}

void traceroute(char *hostname,int probes,int time_to_live,int time_between_probes)
{
    int sockfd, ttl = time_to_live, timeout = 1, flag = 1, i, recv_len;
    clock_t start, end;
    char packet[PACKET_SIZE];
    struct sockaddr_in addr_con;
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    struct iphdr *ip = (struct iphdr *)packet;
    struct icmphdr *icmp = (struct icmphdr *)(packet + sizeof(struct iphdr));
    struct sockaddr_in r_addr;
    socklen_t len = sizeof(r_addr);

    memset(&addr_con, 0, sizeof(addr_con));
    addr_con.sin_family = AF_INET;

    char *ip_addr = dns_lookup(hostname, &addr_con);

    printf("Tracerouting to %s (%s), %d hops max\n", hostname, ip_addr, MAX_HOPS);

    
    for (i = 1; i <= MAX_HOPS; ++i)
    {
        for(int j = 1;j<=probes;++j)
        {

            // starting the clock here because, we are considering the time taken to process the headers as mentioned in question
            start = clock();

            setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
            memset(packet, 0, sizeof(packet));
            icmp->type = ICMP_ECHO;
            icmp->code = 0;
            icmp->un.echo.id = 0;
            icmp->un.echo.sequence = i;
            ip->saddr = 0;
            ip->daddr = addr_con.sin_addr.s_addr;
            ip->version = 4;
            ip->ihl = 5;
            ip->ttl = ttl;
            ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
            ip->protocol = IPPROTO_ICMP;
            icmp->checksum = 0;
            icmp->checksum = checksum((unsigned short *)icmp, sizeof(struct icmphdr));
            ttl++;

            if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
            {
                perror("socket");
                exit(EXIT_FAILURE);
            }

            if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag)) < 0)
            {
                perror("setsockopt");
                exit(EXIT_FAILURE);
            }

            struct pollfd count_down;
            count_down.fd = sockfd;
            count_down.events = POLLIN;

            if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr_con, sizeof(addr_con)) <= 0)
            {
                perror("sendto");
                continue;
            }

            int retval = poll(&count_down, 1, 5000); // wait for 5 seconds

            if (retval > 0)
            {
                if ((recv_len = recvfrom(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&r_addr, &len)) < 0)
                {
                    perror("recvfrom");
                    continue;
                }
                end = clock();
            }
            else if (retval < 0)
            {
                perror("poll");
                close(sockfd);
                continue;
            }
            else
            {
                printf("%d\t* * *\n", i);
                close(sockfd);
                continue;
            }

            if (recv_len > 0)
            {
                double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
                struct sockaddr_in *s = (struct sockaddr_in *)&r_addr;
                printf("%d\t", i);
                printf("%s", inet_ntoa(r_addr.sin_addr));
                printf("\t%.3f ms\n", time_taken * 1000.0);
                if (s->sin_addr.s_addr == addr_con.sin_addr.s_addr)
                {
                    printf("\nTrace Completed\n");
                    break;
                }
            }

            close(sockfd);
        }
    }

    free(ip_addr);
    
}


void ping_net_info(char* hostname,int probe,int time_between_probes)
{
    
}

int main(int argc, char *argv[])
{
    if(argc<4){
         printf("Usage: %s <host> <probe> <time-difference>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int probe = atoi(argv[2]);
    int time_between_probes = atoi(argv[3]);
    ping_net_info(argv[1],probe,time_between_probes);

    return 0;
}
