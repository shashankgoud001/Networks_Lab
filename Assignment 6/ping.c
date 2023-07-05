#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){

    int fd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if (fd==-1) {
        die("%s",strerror(errno));
    }
    
    return 0;
}