#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SIZE 500
#define BUF_SIZE 20
int main(){
   

    
    int sockfd;
    struct sockaddr_in server_address;

    int i,flag=0,temp,exit_flag=0,f;
    char buf[BUF_SIZE],req[SIZE];

    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Unable to create socket\n");
        exit(0);
    }

    server_address.sin_family = AF_INET;
    inet_aton("127.0.0.1",&server_address.sin_addr);
    server_address.sin_port = htons(20000);

    if((connect(sockfd,(struct sockaddr *) &server_address,sizeof(server_address))<0)){
        perror("Unable to connect to server\n");
        exit(0);
    }
    printf("Connected to the server\n");
    // for(i=0; i < BUF_SIZE; i++) buf[i] = '\0';
    // for(i=0; i < SIZE; i++) req[i] = '\0';
    while(1){
    printf("Please enter the expression to evaluate and NOTE to enter @ at the end of the expression : \n");
    // scanf("%[^\n]s",buf);
    bzero(buf,BUF_SIZE);
    bzero(req,SIZE);
        flag=0;
        exit_flag = 0;
        fgets(buf,BUF_SIZE,stdin);

        if(flag==0&&(strlen(buf)>=2&&buf[0]=='-'&&buf[1]=='1')){
            exit_flag = 1;
            // printf("EXIT FLAG\n");
            break;
        }
    while((f = send(sockfd,buf,strlen(buf)+1,0))>0){
        // printf("%s\t%lu\n",buf,strlen(buf));
        // printf("%c\n",buf[strlen(buf)-2]);
        if(flag==0&&(strlen(buf)>=2&&buf[0]=='-'&&buf[1]=='1')){
            exit_flag = 1;
            // printf("EXIT FLAG\n");
            break;
        }
        // printf("flag  : %d\n",f);
        // printf("I AM NOT ABOVE\n")
        temp = strlen(buf);
        // printf("%s\n",buf);
        i = 0;
            while(i<temp){
              if(buf[i]=='@'){
                flag = 1;
                break;
              }
              // printf("%c ",buf[i]);
              buf[i]='\0';
              i++;
            }
        if(i<temp) break;
        flag = 1;
        fgets(buf,BUF_SIZE,stdin);

        if(flag==0&&(strlen(buf)>=2&&buf[0]=='-'&&buf[1]=='1')){
            exit_flag = 1;
            // printf("EXIT FLAG\n");
            break;
        }
     }
    //  printf("I AM FREE\n");
    if(exit_flag) break;
    // printf("I AM AFTER FREE\n");
    // getchar();
    // strcat(req,"\0");
        while((temp = recv(sockfd,buf,BUF_SIZE,0))>0){
            strcat(req,buf);
            // printf("%s\t%d\n",buf,temp);
            i = 0;
            while(i<temp){
              // printf("%c ",buf[i]);
              if(buf[i]=='@'){
                flag = 1;
                break;
              }
              buf[i]='\0';
              i++;
            }
            if(i<temp) break;
          }
          if(temp==0){
            exit_flag = 1;
            // printf("I AM RECEIVE\n");
            break;
          }
    i = 0;
    temp = strlen(req);
    while(i<temp&&req[i]!='@'){
        printf("%c",req[i]);
        i++;
    }
    printf("\n");
    // printf("%s\n",req);

    }   
    // printf("I AM ABOUT TO DISCONNECT FROM SERVER\n");
    close(sockfd);
    printf("Disconnected from the server\n");
    return 0;
}