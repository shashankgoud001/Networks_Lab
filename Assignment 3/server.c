#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h> 
#include <time.h>
#define SIZE 100
#define MAX_SIZE 500
#define BUF_SIZE 40

int send_Message(int newsockfd,char buf[],char command[]){
	strcat(command,"\0");
	int l = strlen(command),i=0,j=0,c=0;
	while(j<l){
		bzero(buf,BUF_SIZE);
		i=0;
		while(i<BUF_SIZE&&j<=l){
			buf[i++]=command[j++];
		}
		if(j>=l) c +=send(newsockfd,buf,strlen(buf)+1,0);
		else c += send(newsockfd,buf,strlen(buf),0);
	}
    return c;
}

int send_Message_util(int newsockfd,char buf[],char command[]){
	strcat(command,"\0");
	bzero(buf,BUF_SIZE);
	int l = strlen(command),i=0,j=0,c=0;
	while(j<l){
		i=0;
		while(i<BUF_SIZE&&j<l){
			buf[i++]=command[j++];
		}
		c+=send(newsockfd,buf,strlen(buf),0);
	}
    return c;
}

int recieve_Message(int newsockfd,char buf[],char command[]){
	bzero(command,MAX_SIZE);
	bzero(buf,BUF_SIZE);
	int t,c=0;
	while((t=recv(newsockfd,buf,BUF_SIZE,0))>0){
		strcat(command,buf);
        c+=t;
		if(buf[t-1]=='\0') break;
	}
    return c;
}


void send_date(int newsockfd,char buf[],char command[]){
        char day[3],month[3],year[5];
        char sec[3],min[3],hour[3];
        time_t a;
        a = time(NULL);
        struct tm b = *localtime(&a);
        sprintf(day,"%d",b.tm_mday);
        sprintf(month,"%d",b.tm_mon+1);
        sprintf(year,"%d",b.tm_year+1900);
        
        strcpy(command,"DATE :  ");
        strcat(command,day);
        strcat(command," - ");
        strcat(command,month);
        strcat(command," - ");
        strcat(command,year);

        strcat(command,"\nTIME :  ");
        
        sprintf(hour,"%d",b.tm_hour);
        sprintf(min,"%d",b.tm_min);
        sprintf(sec,"%d",b.tm_sec);

        strcat(command,hour);
        strcat(command," : ");
        strcat(command,min);
        strcat(command," : ");
        strcat(command,sec);
        strcat(command,"\n");

        send_Message(newsockfd,buf,command);
}

void send_Load(int newsockfd,char buf[],char command[]){
    int x = rand()%100+1;
    sprintf(command,"%d",x);
    send_Message(newsockfd,buf,command);
    printf("Load sent: %d\n",x);
}


int main(int argc,char* argv[])
{
	srand(time(0));
	int			sockfd, newsockfd ; 
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i,x;
	char buf[SIZE],command[MAX_SIZE];	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(atoi(argv[1]));

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 

	while (1) {

		 bzero(command,MAX_SIZE);

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		recieve_Message(newsockfd,buf,command);
		if(strcmp(command,"Send Load")==0){
			send_Load(newsockfd,buf,command);
		}else if(strcmp(command,"Send Time")==0){
			send_date(newsockfd,buf,command);
		}
		close(newsockfd);
	}
	return 0;
}
			

