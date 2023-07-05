#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUF_SIZE 45
#define SIZE 100
#define MAX_SIZE 500
#define PORT_NO 20000

void send_Message(int newsockfd,char buf[],char command[]){
	strcat(command,"\0");
	bzero(buf,BUF_SIZE);
	int l = strlen(command),i=0,j=0;
	while(j<=l){
		i=0;
		bzero(buf,BUF_SIZE);
		while(i<BUF_SIZE&&j<=l){
			buf[i++]=command[j++];
		}
		if(j>=l) send(newsockfd,buf,strlen(buf)+1,0);
		else send(newsockfd,buf,strlen(buf),0);
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

int main()
{
	int	sockfd ;
	struct sockaddr_in	serv_addr;

	int i,j,k,t=1;
	char buf[SIZE];
	char command[MAX_SIZE];
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	// inet_aton("", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(PORT_NO);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

    recieve_Message(sockfd,buf,command); // LOGIN
	// recv(sockfd, buf, BUF_SIZE, 0);	
	printf("%s", command);	
	// bzero(buf,BUF_SIZE);	
	scanf("%s",buf);	// username
	getchar();
	// strcat(buf,"\0");
	send(sockfd, buf, strlen(buf)+1, 0);
	recieve_Message(sockfd,buf,command); 	// FOUND OR NOT-FOUND
	if(strcmp(buf,"FOUND")==0){
		
		while(1){
			
			printf("Enter the command to be sent to server : ");
			fgets(command,MAX_SIZE,stdin);
			command[strlen(command)-1]='\0';
			strcat(command,"\0");
			k=strlen(command);
			if(k>3&&(command[0]=='e'&&command[1]=='x'&&command[2]=='i'&&command[3]=='t')){
				printf("Closing the connection\n");
				break;
			}
			i=0;j=0;
			send_Message(sockfd,buf,command);
			
			i=0;j=0;
			recieve_Message(sockfd,buf,command);
			if(strcmp(command,"$$$$")==0){
				printf("Invalid Command\n");
			}else if(strcmp(command,"####")==0){
				printf("Error in running the code\n");
			}else{
				printf("%s\n",command);
			}
			// t = recv(sockfd,buf,BUF_SIZE,0);
			// printf("t : %d\n",t);
			// printf("%s\n",buf);
			// printf("FINALLY REACHED\n");
			// printf("%s\n",command);
		}

	}else{
		printf("Invalid username\n");
	}
	close(sockfd);
	return 0;

}

