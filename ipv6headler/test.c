#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char **argv){
   int sockfd;
   struct sockaddr_in6 *sendaddr, *recvaddr;
   struct addrinfo *ai, hints;
   if((sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMP)) == -1)
   {
      perror("IN SOCKET()");
      exit(1);
   }
   const int on = 1;
   setsockopt(sockfd, IPPROTO_IPV6, IPV6_RECVHOPOPTS, &on, sizeof(on));
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET6;
   if((getaddrinfo("fe80:0:0:0:6f53:bf9a:62de:e195", NULL, &hints, &ai)) != 0)
   {
      perror("IN SOCKET()");
      exit(1);
   }
   recvaddr = (struct sockaddr_in6 *)ai->ai_addr;
   char addr6[46];
   printf("%s\n", inet_ntop(AF_INET6, recvaddr->sin6_addr.s6_addr, addr6, INET6_ADDRSTRLEN));
}