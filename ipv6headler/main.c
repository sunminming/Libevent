#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

int initopt(void *optbuf){
   void *databuf;
   char *buf = "a";
   socklen_t myoffset = inet6_opt_init(NULL, 0);
   myoffset = inet6_opt_append(NULL, 0, myoffset, 5, 2, 2, &databuf);
   int tempoffset = inet6_opt_set_val(databuf, 0, buf, 2);
   myoffset = inet6_opt_finish(NULL, 0, myoffset);
   
   socklen_t optlen = myoffset;
   databuf = NULL;
   myoffset = inet6_opt_init(optbuf, optlen);
   myoffset = inet6_opt_append(optbuf, optlen, myoffset, 5, 2, 2, &databuf); 
   tempoffset = inet6_opt_set_val(databuf, 0, buf, 2);
   myoffset = inet6_opt_finish(optbuf, optlen, myoffset);
   
   return myoffset;
}

int main(int argc, char **argv){
   int sockfd;
   struct msghdr my_msg;
   struct cmsghdr cmsg;
   struct sockaddr_in6 *senfaddr;
   struct addrinfo *ai, hints;
   if((sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_IPV6)) == -1)
   {
      perror("IN SOCKET()");
      exit(1);
   }
   const int on = 1;
   setsockopt(sockfd, IPPROTO_IPV6, IPV6_RECVHOPOPTS, &on, sizeof(on));
   
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET6;
   if((getaddrinfo("0:0:0:0:0:0:0:1", "0", NULL, &ai)) != 0)
   {
      perror("IN SOCKET()");
      exit(1);
   }
   senfaddr = (struct sockaddr_in6 *)ai->ai_addr;
   
   void *optbuf = malloc(1024);
   memset(optbuf, 0, 1024);
   int optlen = initopt(optbuf);
   
}