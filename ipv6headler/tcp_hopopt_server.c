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
char buf[] = "a";

socklen_t initopt(void *optbuf)
{
   void *databuf;
   
   socklen_t tempoffset, optlen;

   //first traversal for computing expected length of hop-by-hop option
   socklen_t myoffset = inet6_opt_init(NULL, 0);//2
   myoffset = inet6_opt_append(NULL, 0, myoffset, 5, 2, 2, &databuf);//6
   myoffset = inet6_opt_finish(NULL, 0, myoffset);//8

   //second traversal for constructing
   optlen = myoffset;
   myoffset = inet6_opt_init(optbuf, optlen);//2
   myoffset = inet6_opt_append(optbuf, optlen, myoffset, 5, 2, 2, &databuf);//6
   tempoffset = inet6_opt_set_val(databuf, 0, buf, 2);//6
   myoffset = inet6_opt_finish(optbuf, optlen, myoffset);//8
   
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
   setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
   listen(listenfd, 1);

   optbuf = malloc(MAXOPTLEN);
   memset(optbuf, 0, MAXOPTLEN);
   //construct hop-by-hop option
   optlen = initopt(optbuf);

   while(1)
   {
      len = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);

      //set ipv6 hop-by-hop extension headler in all packet sent from connfd
	   if((setsockopt(connfd, IPPROTO_IPV6, IPV6_HOPOPTS, optbuf, optlen)) == -1)
	   {
	      perror("SETSOCKOPT");
		   exit(1);	  
	   }
     
	   send(connfd, "TEST", 5, 0);
      close(connfd);
   }
   free(optbuf);
}