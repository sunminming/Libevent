#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>

#define MAXLINE 1024

void mygetaddinfo(struct sockaddr_in6 *servaddr, char *addr)
{
   struct addrinfo *ai, *hints;
   memset(hints, 0, sizeof(struct addrinfo));
   hints->ai_family = AF_INET6;
   if((getaddrinfo(addr, "9999", hints, &ai)) != 0)
   {
      perror("IN GETADDRINFO()");
      exit(1);
   }
   *servaddr = *(struct sockaddr_in6 *)ai->ai_addr;
   servaddr->sin6_scope_id = 2;
}

int main(int argc, char **argv)
{
	int sockfd, n;
	struct sockaddr_in6 servaddr;
	char recvline[MAXLINE];
   socklen_t len;

	if (argc != 2)
	{
      printf("INPUT ADDRESS!");
      exit(1);
   }

	if ( (sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
      perror("SOCKET");
      exit(1);
   }
   
   const int on = 1;
   setsockopt(sockfd, IPPROTO_IPV6, IPV6_RECVHOPOPTS, &on, sizeof(on));
	bzero(&servaddr, sizeof(servaddr));

   mygetaddinfo(&servaddr, argv[1]);

   len = sizeof(servaddr);
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
   {   
      perror("CONNEXT");
      exit(1);
   }

	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
      {   
         perror("FPUTS");
         exit(1);
      }
	}
}