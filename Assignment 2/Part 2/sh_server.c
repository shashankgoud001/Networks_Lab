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
#define BUF_SIZE 40
#define PORT_NO 20000

void send_Message(int newsockfd,char buf[],char command[]){
	strcat(command,"\0");
	int l = strlen(command),i=0,j=0;
	while(j<l){
		bzero(buf,BUF_SIZE);
		i=0;
		while(i<BUF_SIZE&&j<=l){
			buf[i++]=command[j++];
		}
		// printf("%s\n",buf);
		if(j>=l) send(newsockfd,buf,strlen(buf)+1,0);
		else send(newsockfd,buf,strlen(buf),0);
		// printf("sent :)\n");
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


int verifyClient(char user[]){
	FILE *file;
    file = fopen("users.txt", "r");

    if (file == NULL) {
        printf("Error in opening the file users.txt\n");
        return 0;
    }

    char line[50];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {  // Read one line at a time
		if(line[strlen(line)-1]=='\n'){
			line[strlen(line)-1]='\0';
		}
        if (strcmp(line, user)==0) { 
            printf("Username found\n");
			fclose(file);
			return 1;
        }
    }

    if (!found) {
        printf("Username not found\n");
    }

    fclose(file);
	return 0;
}



int check(char command[]){
	int l = strlen(command),i=0;
	while(i<l&&command[i]==' '){
		i++;
	}
	if(command[i]=='p'){
		if(command[i+1]=='w'){
			if(command[i+2]=='d'){
				// printf("Came HERE TO BEFORE PWD\n");
				return 1;
				// printf("CAME HERE TO AFTER PWD\n");
			}
		}
	}else if(command[i]=='d'){
			if(command[i+1]=='i'){
				if(command[i+2]=='r'){
					// printf("LLLLL\n");
					if(command[i+3]==' '||command[i+3]=='\0') return 2;
					// printf("YO YO YO \n");
				}
			}
	}else if(command[i]=='c'){
			if(command[i+1]=='d'){
				// printf("UMM\n");
				if(command[i+2]==' '||command[i+2]=='\0') return 3;
				// printf("Dam\n");
			}
	}
	// printf("WELL\n");
	return -1;
}

void PWD_(int newsockfd,char buf[],char command[]){
	char cwd[MAX_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		// printf("WHY AM I HERE ?\n");
		strcpy(cwd,"####");
		send(newsockfd,cwd,strlen(cwd)+1,0);
		return;
	}
	// printf("%s\n",cwd);
	send_Message(newsockfd,buf,cwd);
}

void DIR_(char command[],int newsockfd,char buf[]){
	int l = strlen(command);
	int i = 0;
	char* path = strtok(command, " ");
	if(path != NULL) path = strtok(NULL, " ");
	if(path==NULL){
		path = (char*)malloc(10*sizeof(char));
		path[0] = '.';
		path[1]='\0';
		// strcpy(command,"."); 
	}else{
		
	}
		struct dirent *de;

    DIR *dr = opendir(path); 
  
    if (dr == NULL)
    { 
		char cwd[10];
		strcpy(cwd,"####");
		send(newsockfd,cwd,strlen(cwd)+1,0);
		return;
    }  
    while ((de = readdir(dr)) != NULL) {
		strcat(de->d_name,"\n");
		send_Message_util(newsockfd,buf,de->d_name);
	}
	buf[0]='\0';
	send(newsockfd,buf,strlen(buf)+1,0);
}

void CD_(char command[],int newsockfd,char buf[]){
	int l = strlen(command);
	// printf("%s\n",command);
	char* path = strtok(command, " ");
	// printf("%s\n",path);
	if(path != NULL) path = strtok(NULL, " ");
	if(path==NULL){
		path = (char*)malloc(SIZE*sizeof(char));
		strcpy(path,"/home/");
		strcat(path,getenv("USER"));
		
	}else{
	
	}
		struct dirent *de;
	// printf("------\n");
	int ch=chdir(path);
	// printf("+++++++\n");
	if(ch<0){
		char cwd[10];
		bzero(cwd,10);
		strcpy(cwd,"####");
		// printf("I came here mm\n");
		send_Message(newsockfd,buf,cwd);
		// printf("I jumped here\n");
		return;
	}
    char temp[100];
	bzero(temp,100);
	// printf("Changed the directory successfully\n");
	strcpy(temp,"Changed the directory successfully");
	send_Message(newsockfd,buf,temp);
	return;
}

void perform_service(int newsockfd,char buf[],char command[]){

	switch (check(command))
	{
	case 1:
		PWD_(newsockfd,buf,command);
		break;
	case 2:
		DIR_(command,newsockfd,buf);
		break;
	case 3:
		CD_(command,newsockfd,buf);
		break;
	default:
		bzero(command,20);
		strcpy(command,"$$$$");
		send_Message(newsockfd,buf,command);
		break;
	}
}

int main()
{
	int			sockfd, newsockfd ; 
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i,j,k,f,t=1;
	char buf[SIZE],command[MAX_SIZE],*temp = (char*)malloc(sizeof(char)*MAX_SIZE);		

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(PORT_NO);


	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		if (fork() == 0) {
			close(sockfd);
			bzero(buf,BUF_SIZE);
			bzero(command,MAX_SIZE);
			strcpy(buf,"LOGIN:\0");
			// strcpy(command,)
			send(newsockfd, buf, strlen(buf)+1, 0); //login
			 
			recieve_Message(newsockfd,buf,command); //username
			
			if(verifyClient(command)){
				bzero(buf,BUF_SIZE);
				strcpy(buf,"FOUND\0");

				send(newsockfd,buf,strlen(buf)+1,0); // sending found
				

				while(1){
					
					recieve_Message(newsockfd,buf,command);	//receiveing command

					// printf("WAYYYYYYY above herererererere\n");
					// printf("%s\n",command);

					perform_service(newsockfd,buf,command);
					
					// printf("my work is done\n");
				}
			}else{
				bzero(buf,BUF_SIZE);
				strcpy(buf,"NOT-FOUND\0");
				send(newsockfd,buf,strlen(buf)+1,0);
			}
			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
	return 0;
}
			


// #include <stdio.h>
// #include <stdlib.h>

// int main()
// {
// 	printf("%s", getenv("USER"));
// 	return 0;
// }