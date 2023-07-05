#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


#define SIZE 500
#define CLIENT_LIMIT 10

int main(){
    int sockfd,newsockfd;
    int clilen;
    struct sockaddr_in client_address,server_address;
    int i;
    char buf[SIZE];
    char day[3],month[3],year[5];
    char sec[3],min[3],hour[3];
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("CANNOT CREATE SOCKET\n");
        exit(0);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(20000);

    if(bind(sockfd, (struct sockaddr *) &server_address,sizeof(server_address))<0){
        perror("Unable to bind local address\n");
        exit(0);
    }
    listen(sockfd,CLIENT_LIMIT);
    while(1){
        clilen = sizeof(client_address);
        newsockfd = accept(sockfd,(struct sockaddr *) &client_address, &clilen);
        if(newsockfd<0){
            perror("ACCEPT ERROR\n");
            exit(0);
        }

        //
        
        time_t a;
        a = time(NULL);
        struct tm b = *localtime(&a);
        sprintf(day,"%d",b.tm_mday);
        sprintf(month,"%d",b.tm_mon+1);
        sprintf(year,"%d",b.tm_year+1900);
        
        strcpy(buf,"DATE :  ");
        strcat(buf,day);
        strcat(buf," - ");
        strcat(buf,month);
        strcat(buf," - ");
        strcat(buf,year);

        strcat(buf,"\nTIME :  ");
        
        sprintf(hour,"%d",b.tm_hour);
        sprintf(min,"%d",b.tm_min);
        sprintf(sec,"%d",b.tm_sec);

        strcat(buf,hour);
        strcat(buf," : ");
        strcat(buf,min);
        strcat(buf," : ");
        strcat(buf,sec);
        strcat(buf,"\n");
        
        //
        send(newsockfd,buf,strlen(buf)+1,0);
        recv(newsockfd,buf,100,0);
        printf("%s",buf);
        close(newsockfd);
    }
    return 0;
}