// C program to display hostname
// and IP address
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry)
{
	if (hostentry == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
	if (NULL == IPbuffer)
	{
		perror("inet_ntoa");
		exit(1);
	}
}
int validate_number(char *str) {
   while (*str) {
      if(!isdigit(*str)){ //if the character is not a number, return
         return 0;
      }
      str++; //point to next character
   }
   return 1;
}
int validate_ip(char *ip) { //check whether the IP is valid or not
   int i, num, dots = 0;
   char *ptr;
   if (ip == NULL)
      return 0;
      ptr = strtok(ip, "."); //cut the string using dor delimiter
      if (ptr == NULL)
         return 0;
   while (ptr) {
      if (!validate_number(ptr)) //check whether the sub string is
         return 0;
         num = atoi(ptr); //convert substring to number
         if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, "."); //cut the next part of the string
            if (ptr != NULL)
               dots++; //increase the dot count
         } else
            return 0;
    }
    if (dots != 3) //if the number of dots are not 3, return false
       return 0;
      return 1;
}
// Driver code
int main(int argc,char **argv)
{
	char hostbuffer[256];
	char *IPbuffer;
	struct hostent *host_entry;
	int hostname;
    if(validate_ip(argv[1])){
        IPbuffer = argv[1];
    }else{
    	host_entry = gethostbyname(argv[1]);
	    checkHostEntry(host_entry);
	    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    }
	

	printf("Hostname: %s\n", argv[1]);
	printf("Host IP: %s\n", IPbuffer);

	return 0;
}
