#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

int CreateRawSocket(int protocol_to_sniff)
{
    int rawsock;
    if((rawsock = socket(PF_PACKET,SOCK_RAW,htons(protocol_to_sniff)))==-1)
    {
        perror("Error creating raw socket: ");
        exit(-1);
    }
    return rawsock;
}

int main(){
    int raw;
    unsigned char packet_buffer[2048];
    int len;
    int packets_to_sniff;
    struct sockaddr_ll packet_info;
    int packet_info_size = sizeof(packet_info_size);
    return 0;
}