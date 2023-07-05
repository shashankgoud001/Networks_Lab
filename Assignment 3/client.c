#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h> 
#define SIZE 100
#define MAX_SIZE 500
#define BUF_SIZE 45

void send_Message(int newsockfd,char buf[],char command[]){
	strcat(command,"\0");
	int l = strlen(command),i=0,j=0;
	while(j<l){
		bzero(buf,BUF_SIZE);
		i=0;
		while(i<BUF_SIZE&&j<=l){
			buf[i++]=command[j++];
		}
		if(j>=l) send(newsockfd,buf,strlen(buf)+1,0);
		else send(newsockfd,buf,strlen(buf),0);
	}
}

void send_Message_util(int newsockfd,char buf[],char command[]){
	strcat(command,"\0");
	bzero(buf,BUF_SIZE);
	int l = strlen(command),i=0,j=0;
	while(j<l){
		i=0;
		while(i<BUF_SIZE&&j<l){
			buf[i++]=command[j++];
		}
		send(newsockfd,buf,strlen(buf),0);
	}
}

void recieve_Message(int newsockfd,char buf[],char command[]){
	bzero(command,MAX_SIZE);
	bzero(buf,BUF_SIZE);
	int t;
	while((t=recv(newsockfd,buf,BUF_SIZE,0))>0){
		strcat(command,buf);
		if(buf[t-1]=='\0') break;
	}
}

int main(int argc,char* argv[])
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[SIZE],command[MAX_SIZE];


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(atoi(argv[1]));

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
    int x;
    recieve_Message(sockfd,buf,command);
    printf("%s\n",command);
	close(sockfd);
	return 0;

}

