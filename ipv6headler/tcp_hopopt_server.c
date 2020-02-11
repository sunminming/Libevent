#include <sys/socket.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define MAXOPTLEN 1024

socklen_t initopt(void *optbuf)
{
   void *databuf;
   char buf[] = "a";
   socklen_t tempoffset, optlen;
   
   socklen_t myoffset = inet6_opt_init(NULL, 0);
   myoffset = inet6_opt_append(NULL, 0, myoffset, 5, 2, 2, &databuf);
   myoffset = inet6_opt_finish(NULL, 0, myoffset);
   
   optlen = myoffset;
   myoffset = inet6_opt_init(optbuf, optlen);
   myoffset = inet6_opt_append(optbuf, optlen, myoffset, 5, 2, 2, &databuf); 
   tempoffset = inet6_opt_set_val(databuf, 0, buf, 2);
   myoffset = inet6_opt_finish(optbuf, optlen, myoffset);
   
   return myoffset;
}

int main(int argc, char **argv)
{
   int listenfd, connfd;
   socklen_t len, optlen;
   struct sockaddr_in6 servaddr, cliaddr;
   void *optbuf;
   
   listenfd = socket(AF_INET6, SOCK_STREAM, 0);
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin6_family = AF_INET6;
   servaddr.sin6_addr = in6addr_any;
   servaddr.sin6_port = htons(9999);

   bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   const int on = 1;
   setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
   listen(listenfd, 1);

   optbuf = malloc(MAXOPTLEN);
   memset(optbuf, 0, MAXOPTLEN);
   optlen = initopt(optbuf);

   while(1)
   {
      len = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);

	   if((setsockopt(connfd, IPPROTO_IPV6, IPV6_HOPOPTS, optbuf, 8)) == -1)
	   {
	      perror("SETSOCKOPT");
		   exit(1);	  
	   }
     
	   send(connfd, "TEST", 5, 0);
      close(connfd);
   }
   free(optbuf);
}