

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mysocket.h"

int main()
{
	int			sockfd, newsockfd ; 
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];
	if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (my_bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	my_listen(sockfd, 5);

	
	// while (1) {

		
		clilen = sizeof(cli_addr);
		newsockfd = my_accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}


		
		
		strcpy(buf,"Message from server1");
		my_send(newsockfd, buf, strlen(buf) + 1, 0);

		
		my_recv(newsockfd, buf, 100, 0);
		printf("%s\n", buf);

		strcpy(buf,"Message from server2");
		my_send(newsockfd, buf, strlen(buf) + 1, 0);

		
		my_recv(newsockfd, buf, 100, 0);
		printf("%s\n", buf);

		my_close(newsockfd);
	// }
    my_close(sockfd);
	return 0;
}
			

