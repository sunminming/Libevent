#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sock_basis.h"
#include "icmpv6_basis.h"

//the interface index
int ifindex;
//the iv6 address of interface
struct in6_addr ifaddr;
//the mac of interface
struct ether_addr macaddr;

int check_arg(int argc, char **argv)
{
   if(argc != 2) return 0;
   else
   {
      ifindex = if_nametoindex(argv[1]);
      if(ifindex == 0) return 0;
      memset(&ifaddr, 0, sizeof(struct in6_addr));
      int len = sizeof(struct in6_addr);
      if(addr_fromname(AF_INET6, argv[1], &ifaddr, &len) == 0) return 0;
      return mac_fromname(argv[1], &macaddr);
   }
}

int main(int argc, char **argv)
{
   if(check_arg(argc, argv) == 0)
   {
      printf("Please input a interface name!\n");
      exit(1);
   }
   int sockfd = init_sock(AF_INET6, IPPROTO_ICMPV6, NULL);
   if(sockfd == -1)
   {
      perror("In init_sock()");
      exit(1);
   }
   
   if(join_groups(AF_INET6, sockfd, NULL, 0) == 0)
   {
      perror("In join_groups()");
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
         //fe80::6f53:bf9a:62de:e195
         //fe80::8ccb:4b57:7e4e:bc95
         if(send_ns(sockfd, buf + be) <= 0) perror("Error in send NS");
         continue;
      }

      if(memcmp(buf, "na", 2) == 0)
         recv_ns(sockfd);
   }
}