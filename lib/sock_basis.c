#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

extern const uint8_t ipv6_multicast_prefix[13] = {
   0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x01, 0xff
};

int init_sock(int family, int protocol, void *parameter)
{
   int sockfd;
   switch (protocol)
   {
   case IPPROTO_ICMPV6:
      sockfd = socket(family, SOCK_RAW, protocol);
      break;
   
   default:
      break;
   }
   return sockfd;
}

int join_groupaddr6(int sockfd, char *addr, int interface)
{
   //ff02::1
   struct ipv6_mreq mreq6;
   struct in6_addr sin6_addr;
   if(inet_pton(AF_INET6, addr, (void *)&sin6_addr) == 0) return 0;

   memcpy(&mreq6.ipv6mr_multiaddr, &sin6_addr, sizeof(struct in6_addr));
   mreq6.ipv6mr_interface = interface;
   if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) == -1) return 0;
   else return 1;
}

socklen_t ipaddr(int family, char *addr, int scope_id, void *res)
{
   struct addrinfo *res_ai, *hints;
   res_ai = (struct addrinfo *)malloc(sizeof(struct addrinfo));
   hints = (struct addrinfo *)malloc(sizeof(struct addrinfo));    
   memset(hints, 0, sizeof(struct addrinfo));
   memset(res_ai, 0, sizeof(struct addrinfo));

   if(family == AF_INET6)
   {
      struct sockaddr_in6 *loaddr;
      loaddr = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
      memset(loaddr, 0, sizeof(struct sockaddr_in6));
      
      hints->ai_family = AF_INET6;
      if((getaddrinfo(addr, "0", hints, &res_ai)) == 0)
      {
         //res_ai->ai_addr->sin6_scope_id = scope_id;
         loaddr = (struct sockaddr_in6 *)res_ai->ai_addr;
         loaddr->sin6_scope_id = scope_id;
         *(struct sockaddr_in6 *)res = *loaddr;
         socklen_t res_len = res_ai->ai_addrlen;
         free(res_ai);
         return res_len;
      }
   }
}

int group_join(int family, int sockfd, char **parameter, int parac)
{
   if(family == AF_INET6)
   {
      //join link scope
      join_groupaddr6(sockfd, "ff02::1", 2);
      //join parameter addr
      for(int i = 0; i < parac; ++i)
      {
         if(join_groupaddr6(sockfd, parameter[i], 2) == 0)
         {
            printf("Error %s when join the group %s\n", strerror(errno), parameter[i]);
            return 0;
         }
         else continue;
      }
      return 1;
   }
}

int to_solicitation_node_groupaddr(char *tar, struct in6_addr *res)
{
   if(inet_pton(AF_INET6, tar, (void *)res) != 1) return 0;
   memcpy(res->s6_addr, ipv6_multicast_prefix, 13);
   return 1;
}