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
#include <poll.h>
#define SIZE 100
#define MAX_SIZE 500
#define BUF_SIZE 50

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

void recieveLoadfromServer(int sockfd,int* s,char buf[],char command[],char s_IP[]){
    strcpy(command,"Send Load");
    send_Message(sockfd,buf,command);
    recieve_Message(sockfd,buf,command);
    printf("Load recieved from Server 1 with IP : %s and Load : %s\n",s_IP,command);
    *s = atoi(command);
}

int main(int argc,char* argv[])
{
    int	sockfd_s1 ;
	struct sockaddr_in	serv_addr_s1;

	int i,t,flag;
    int r,s1,s2;
	char buf[SIZE],command[MAX_SIZE],s1_IP[20],s2_IP[20];


	
	serv_addr_s1.sin_family	= AF_INET;
    strcpy(s1_IP,"127.0.0.1");
	inet_aton(s1_IP, &serv_addr_s1.sin_addr);
	serv_addr_s1.sin_port	= htons(atoi(argv[2]));

	

////////////////////////////////////////////////////////////////////////////////////////////////////

    int	sockfd_s2 ;
	struct sockaddr_in	serv_addr_s2;





	serv_addr_s2.sin_family	= AF_INET;
    strcpy(s2_IP,"127.0.0.1");
	inet_aton(s2_IP, &serv_addr_s2.sin_addr);
	serv_addr_s2.sin_port	= htons(atoi(argv[3]));


    int			sockfd, newsockfd ; 
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

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
    time_t current_time,prev_time;
    
    struct pollfd fds;
    fds.fd = sockfd;
    fds.events = POLLIN;
    current_time = time(NULL);
    t = 0;
    while(1){
        if(t>0){
            r = poll(&fds, 1, t);
            if(r<0){
                printf("Error\n");
                break;
            }else if(r==0){
                if ((sockfd_s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		            perror("Unable to create socket\n");
		            exit(0);
	            }
               if ((connect(sockfd_s1, (struct sockaddr *) &serv_addr_s1,
						sizeof(serv_addr_s1))) < 0) {
                        perror("Unable to connect to server\n");
                        exit(0);
                }
                recieveLoadfromServer(sockfd_s1,&s1,buf,command,s1_IP);
                close(sockfd_s1);
                if ((sockfd_s2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		            perror("Unable to create socket\n");
		            exit(0);
                }
                if ((connect(sockfd_s2, (struct sockaddr *) &serv_addr_s2,
                            sizeof(serv_addr_s2))) < 0) {
                            perror("Unable to connect to server\n");
                            exit(0);
                }
                recieveLoadfromServer(sockfd_s2,&s2,buf,command,s2_IP);
                close(sockfd_s2);
                t = 5000;
                current_time = time(NULL);
            }else{
                clilen = sizeof(cli_addr);
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
                            &clilen) ;

                if (newsockfd < 0) {
                    printf("Accept error\n");
                    exit(0);
                }
                if(fork()==0){
                    close(sockfd);
                if(s1<s2){
                    if ((sockfd_s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		                perror("Unable to create socket\n");
		                exit(0);
	                }
                    if ((connect(sockfd_s1, (struct sockaddr *) &serv_addr_s1,
						sizeof(serv_addr_s1))) < 0) {
                        perror("Unable to connect to server\n");
                        exit(0);
                    }
                    printf("Sending client request to Server 1 with IP : %s\n",s1_IP);
                    strcpy(command,"Send Time");
                    send_Message(sockfd_s1,buf,command);
                    recieve_Message(sockfd_s1,buf,command);
                    close(sockfd_s1);
                    send_Message(newsockfd,buf,command);
                }else{
                    if ((sockfd_s2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                        perror("Unable to create socket\n");
                        exit(0);
                    }
                    if ((connect(sockfd_s2, (struct sockaddr *) &serv_addr_s2,
						sizeof(serv_addr_s2))) < 0) {
		                perror("Unable to connect to server\n");
		                exit(0);
	                }
                    printf("Sending client request to Server 2 with IP : %s\n",s2_IP);
                    strcpy(command,"Send Time");
                    send_Message(sockfd_s2,buf,command);
                    recieve_Message(sockfd_s2,buf,command);
                    close(sockfd_s2);
                    send_Message(newsockfd,buf,command);
                }
                close(newsockfd);
                exit(0);
                }
                close(newsockfd);
                t = t +( -time(NULL) + current_time)*1000;
                current_time = time(NULL);
            }
        }else{
            if ((sockfd_s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		            perror("Unable to create socket\n");
		            exit(0);
	            }
            if ((connect(sockfd_s1, (struct sockaddr *) &serv_addr_s1,
						sizeof(serv_addr_s1))) < 0) {
                        perror("Unable to connect to server\n");
                        exit(0);
            }
            recieveLoadfromServer(sockfd_s1,&s1,buf,command,s1_IP);
            close(sockfd_s1);
            if ((sockfd_s2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		            perror("Unable to create socket\n");
		            exit(0);
                }
            if ((connect(sockfd_s2, (struct sockaddr *) &serv_addr_s2,
						sizeof(serv_addr_s2))) < 0) {
		                perror("Unable to connect to server\n");
		                exit(0);
            }
            recieveLoadfromServer(sockfd_s2,&s2,buf,command,s2_IP);
            close(sockfd_s2);
            t = 5000;
            current_time = time(NULL);
        }
        

    }
    close(sockfd);
	return 0;
}