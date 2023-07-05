#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#define MAX_SIZE 1024
int main()
{
    int t, f,flag=1;
    char buf[MAX_SIZE];
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    memset(servaddr.sin_zero, '\0', sizeof servaddr.sin_zero);

    /*Initialize size variable to be used later on*/

    struct pollfd fds;
    fds.fd = sockfd;
    fds.events = POLLIN;

    for (int i = 0; i < 5; i++)
    {
        printf("Trying to send a message to server\n");
        sendto(sockfd, "ACK", strlen("ACK"), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        /* Here we are Waiting for 3 seconds before trying to receive another response */
        t = poll(&fds, 1, 3000);

        if (t == -1)
        {
            printf("Error in poll()\n");
        }
        else if (t)
        {
            /*Receive message from server*/
            f = recvfrom(sockfd, buf, MAX_SIZE, 0, NULL, NULL);
            printf("%s\n", buf);
            flag = 0;
            break; // exit the loop as we have received the response
        }
        else
        {
            printf("Did not recieve any data within 3 seconds\n");
        }
    }
    if (flag)
        printf("Timeout exceeded\n");
    
    close(sockfd);
    return 0;
}
