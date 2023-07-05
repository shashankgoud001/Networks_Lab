#ifndef __MYSOCKET_H
#define __MYSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#define SOCK_MyTCP 143

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, struct sockaddr *addr, int addrlen);
int my_listen(int sockfd, int backlog);
int my_accept(int sockfd, struct sockaddr *addr, int *addrlen);
int my_connect(int sockfd, struct sockaddr *addr, int addrlen);
int my_send(int sockfd, void *buf, int len, int flags);
int my_recv(int sockfd, void *buf, int len, int flags);
int my_close(int sockfd);

#endif