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

#define MAXLINE 1024
#define MAXOPTLEN 1024
socklen_t control_len;
char buf[] = "a";

int initopt(void *optbuf)
{
   void *databuf;
   //char *buf = "a";
   socklen_t tempoffset;
   socklen_t myoffset = inet6_opt_init(NULL, 0);
   myoffset = inet6_opt_append(NULL, 0, myoffset, 5, 2, 2, &databuf);
   myoffset = inet6_opt_finish(NULL, 0, myoffset);
   
   socklen_t optlen = myoffset;
   databuf = NULL;
   myoffset = inet6_opt_init(optbuf, optlen);
   myoffset = inet6_opt_append(optbuf, optlen, myoffset, 5, 2, 2, &databuf); 
   tempoffset = inet6_opt_set_val(databuf, 0, buf, 2);
   myoffset = inet6_opt_finish(optbuf, optlen, myoffset);
   
   return myoffset;
}

void init_msg(struct msghdr *mymsg, struct sockaddr_in6 *cliaddr)
{
   void *optbuf;
   optbuf = malloc(1024);
   memset(optbuf, 0, 1024);
   int optlen = initopt(optbuf);
   struct iovec *iov;
   //cmsghdar
   struct cmsghdr *mycmsg;
   char *control = malloc(CMSG_SPACE(optlen));
   memset(control, 0, CMSG_SPACE(optlen));
   mymsg->msg_control = control;
   mymsg->msg_controllen = sizeof(control);
   
   //mycmsg = CMSG_FIRSTHDR(mymsg);
   mycmsg = (struct cmsghdr *)mymsg->msg_control;
   mycmsg->cmsg_len = CMSG_LEN(optlen);
   control_len = mycmsg->cmsg_len;
   mycmsg->cmsg_level = IPPROTO_IPV6;
   mycmsg->cmsg_type = IPV6_HOPOPTS;
   //control mag data
   //unsigned char *control_data = CMSG_DATA(mycmsg);
   memcpy(CMSG_DATA(mycmsg), optbuf, optlen);
   free(optbuf);
   //msg_name is servaddr
   mymsg->msg_name = cliaddr;
   mymsg->msg_namelen = sizeof(struct sockaddr_in6);
   //iov
   iov = malloc(sizeof(struct iovec));
   iov->iov_base = "TEST";
   iov->iov_len = sizeof("TEST");
   mymsg->msg_iov = iov;
   mymsg->msg_iovlen = 1;
   
}

void free_msghdr(struct msghdr *mymsg)
{
   free(mymsg->msg_iov);
   free(mymsg->msg_control);
   free(mymsg);	
}

int main(int argc, char **argv)
{
   int connfd, optlen;
   socklen_t len;
   char buff[MAXLINE];
   struct sockaddr_in6 servaddr, cliaddr;
   struct msghdr *mymsg;
   char *optbuf;

   connfd = socket(AF_INET6, SOCK_DGRAM, 0);
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin6_family = AF_INET6;
   servaddr.sin6_addr   = in6addr_any;
   servaddr.sin6_port   = htons(9999);

   bind(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   const int on = 1;
   setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
   
   optbuf = malloc(MAXOPTLEN);
   memset(optbuf, 0, MAXOPTLEN);
   //construct hop-by-hop option
   optlen = initopt(optbuf);

   
   if((setsockopt(connfd, IPPROTO_IPV6, IPV6_HOPOPTS, optbuf, optlen)) == -1)
	{
	   perror("SETSOCKOPT");
		exit(1);	  
	}
   
   for ( ; ; ) 
   {
      len = sizeof(cliaddr); 
      memset(buff, 0, MAXLINE);
      recvfrom(connfd, buff, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
	   
      sendto(connfd, "Hello!", sizeof("Hello!"), 0, (struct sockaddr *)&cliaddr, len);
      buf[0] = 'b';
      mymsg = malloc(sizeof(struct msghdr));
	   memset(mymsg, 0, sizeof(struct msghdr));
	   init_msg(mymsg, (struct sockaddr_in6 *)&cliaddr);
      sendmsg(connfd, mymsg, 0);
   }
   free_msghdr(mymsg);
}