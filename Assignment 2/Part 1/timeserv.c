#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#define MAX_SIZE 500
int main()
{
    int sockfd, t;
    char buf[MAX_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    int i;
    char day[3], month[3], year[5];
    char sec[3], min[3], hour[3];
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(20000); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\nServer Running....\n");
    while (1)
    {

        len = sizeof(cliaddr);
        t = recvfrom(sockfd, buf, MAX_SIZE, 0, (struct sockaddr *)&cliaddr, &len);

        time_t a;
        a = time(NULL);
        struct tm b = *localtime(&a);
        sprintf(day, "%d", b.tm_mday);
        sprintf(month, "%d", b.tm_mon + 1);
        sprintf(year, "%d", b.tm_year + 1900);

        strcpy(buf, "DATE :  ");
        strcat(buf, day);
        strcat(buf, " - ");
        strcat(buf, month);
        strcat(buf, " - ");
        strcat(buf, year);

        strcat(buf, "\nTIME :  ");

        sprintf(hour, "%d", b.tm_hour);
        sprintf(min, "%d", b.tm_min);
        sprintf(sec, "%d", b.tm_sec);

        strcat(buf, hour);
        strcat(buf, " : ");
        strcat(buf, min);
        strcat(buf, " : ");
        strcat(buf, sec);
        strcat(buf, "\n");
        printf("%s", buf);
        strcat(buf, "\0");
        sendto(sockfd, buf, strlen(buf) + 1, 0, (struct sockaddr *)&cliaddr, len);
    }
    close(sockfd);
    return 0;
}
