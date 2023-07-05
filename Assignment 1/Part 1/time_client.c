#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SIZE 500

int main()
{
    int sockfd;
    struct sockaddr_in server_address;
    int i;
    char buf[SIZE];
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Unable to create socket\n");
        exit(0);
    }
    server_address.sin_family = AF_INET;
    inet_aton("127.0.0.1",&server_address.sin_addr);
    server_address.sin_port = htons(20000);

    if((connect(sockfd,(struct sockaddr *) &server_address, sizeof(server_address)))<0){
        perror("Unable to connect to the server\n");
        exit(0);
    }

    for(i = 0;i<SIZE;++i) buf[i] = '\0';
    recv(sockfd,buf,SIZE,0);
    printf("%s",buf);

    strcpy(buf,"Client Successfully Received Date and Time\n");
    send(sockfd,buf,strlen(buf)+1,0);

    close(sockfd);
    return 0;
}