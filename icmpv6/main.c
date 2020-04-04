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
#include <pthread.h>
#include "sock_basis.h"
#include "icmpv6_basis.h"

//the interface index
int ifindex;
//the flag to indicate whether sned na
int naflag;
//the iv6 address of interface
struct in6_addr ifaddr;
//the mac of interface
struct ether_addr macaddr;

int check_arg(int argc, char **argv)
{
   if(argc < 2) return 0;
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

int opt_check(int argc, char **argv, char *opt)
{
   for(int i = 2; i < argc; ++i)
   {
      if(strcmp(argv[i], opt) == 0) return 1;
      else continue;
   }
   return 0;
}

int main(int argc, char **argv)
{
   //check necessary parameters
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
   
   //block all icmpv6 message
   struct icmp6_filter *nafilter = (struct icmp6_filter *)malloc(sizeof(struct icmp6_filter));
   ICMP6_FILTER_SETBLOCKALL(nafilter);
   if(setsockopt(sockfd, IPPROTO_ICMPV6, ICMP6_FILTER, nafilter, sizeof(struct icmp6_filter)) == -1)
   {
      perror("Set icmp6 filter");
      exit(1);
   }
   free(nafilter);

   //join necessary multiaddress
   if(join_groups(AF_INET6, sockfd, NULL, 0) == 0)
   {
      perror("In join_groups()");
      exit(1);
   }

   //check option parameters
   struct recvns_data *nsdata;
   pthread_t nslisten_tid;

   //a single fd?
   if(opt_check(argc, argv, "offNA") == 0) naflag = 1;//listen but do not send na
   else naflag = 0;

   nsdata = listen_ns(&nslisten_tid);

   //main loop
   char buf[50];
   ssize_t len;
   printf("Loading Completed\n");
   while(1)
   {
      memset(buf, '\0', 50);
      if((len = read(STDIN_FILENO, buf, 50)) <= 0)
      {
         perror("STDIN ERROR");
         exit(1);
      }

      buf[len - 1] = '\0';
      --len; 

      if(memcmp(buf, "ns", 2) == 0)
      {
         int be = 2;
         while(be < len && buf[be] == ' ') ++be;
         //fe80::6f53:bf9a:62de:e195
         //fe80::8ccb:4b57:7e4e:bc95
         //fe80::dea1:27f4:19b3:96ab
         if(be == len) print_ns(nsdata);
         else if(send_ns(sockfd, buf + be) <= 0) perror("Error in send NS");
         continue;
      }

      if(memcmp(buf, "ping", 4) == 0)
      {
         int be = 4;
         while(be < len && buf[be] == ' ') ++be;
         if(be == len) printf("Please input dest ipv6 address\n");
         else if(ping(sockfd, buf + be) <= 0) perror("Error in send echo request");
         continue;
      }
   }
}