#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sock_basis.h"
#include "icmpv6_basis.h"

int check_arg(int argc, char **argv)
{
   int scope_id;
   struct addrinfo *res_ai, *hints;
   struct sockaddr_in6 *loaddr;
   if(argc != 3) return 0;
   else
   {
      scope_id = atoi(argv[2]);
      if(scope_id == 0) return 0;

      res_ai = (struct addrinfo *)malloc(sizeof(struct addrinfo));
      hints = (struct addrinfo *)malloc(sizeof(struct addrinfo));
      loaddr = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
      memset(hints, 0, sizeof(struct addrinfo));
      memset(res_ai, 0, sizeof(struct addrinfo));
      memset(loaddr, 0, sizeof(struct sockaddr_in6));
      
      hints->ai_family = AF_INET6;
      if((getaddrinfo(argv[1], "0", hints, &res_ai)) != 0) return 0;
      else return 1;
   }
}

int main(int argc, char **argv)//argv[1] is the ipv6 address, argv[2] is the scope_id
{
   
   if(check_arg(argc, argv) == 0)
   {
      printf("Please input a ipv6 address and interface id!\n");
      exit(1);
   }
   int sockfd = init_sock(AF_INET6, IPPROTO_ICMPV6, NULL);

   if(group_join(AF_INET6, sockfd, NULL, 0) == 0)
   {
      printf("Join group error!\n");
      exit(1);      
   }

   char buf[50];
   ssize_t len;
   memset(buf, '\0', 50);
   while(1)
   {
      if((len = read(STDIN_FILENO, buf, 50)) <= 0)
      {
         perror("STDIN ERROR");
         exit(1);
      }

      buf[len - 1] = '\0'; 
      if(memcmp(buf, "ns", 2) == 0)
      {
         int be = 2;
         while(buf[be] == ' ') ++be;
         send_ns(sockfd, buf + be);
      }
   }
}