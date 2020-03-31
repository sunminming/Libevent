#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <errno.h>
#include <ifaddrs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const uint8_t ipv6_multicast_prefix[13] = {
   0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x01, 0xff
};

const struct in6_addr allnodes_link = 
{
   {
      0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
   }
};

extern int ifindex;
extern struct in6_addr ifaddr;
extern struct ether_addr macaddr;

void free_allai(struct addrinfo *res)
{
   struct addrinfo *next;
   while(res)
   {
      if(res->ai_canonname) free(res->ai_canonname);
      if(res->ai_addr) free(res->ai_addr);
      next = res->ai_next;
      free(res);
      res = next;
   }
}

int mac_fromname(const char *name, struct ether_addr *res)
{
   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   struct ifreq ifr;
   memset(&ifr, 0, sizeof(struct ifreq));
   memcpy(&(ifr.ifr_name), name, sizeof(name));
   if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0)
   {
      memcpy(&(macaddr.ether_addr_octet), &(ifr.ifr_ifru.ifru_hwaddr.sa_data), ETH_ALEN);
      close(sockfd);
      return 1;
   }
   else
   {
      perror("ioctl");
      close(sockfd);
      return 0;
   }
}

int addr_fromname(const int family, const char *name, void *res, int *len)
{
   if(family == AF_INET6)
   {
      if(*len < sizeof(struct in6_addr)) return 0;
      struct ifaddrs *ifaddr;
      if (getifaddrs(&ifaddr) == -1) return 0;
      while(ifaddr != NULL)
      {
         if(ifaddr->ifa_addr == NULL) continue;
         if((memcmp(ifaddr->ifa_name, name, strlen(name)) == 0) && (ifaddr->ifa_addr->sa_family == AF_INET6))
         {
            struct sockaddr_in6 *temp = (struct sockaddr_in6 *)ifaddr->ifa_addr;
            memcpy(res, &(temp->sin6_addr), sizeof(struct in6_addr));
            *len = sizeof(struct in6_addr);
            return 1;
         }
         ifaddr = ifaddr->ifa_next;
      }
      return 0;
   }
}

int init_sock(const int family, const int protocol, void *parameter)
{
   int sockfd;
   int ttl = 255;
   switch (protocol)
   {
   case IPPROTO_ICMPV6:
      sockfd = socket(family, SOCK_RAW, protocol);
      break;
   
   default:
      break;
   }
   
   return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(int));
}

int join_agroup(const int sockfd, const struct in6_addr *addr)
{
   struct ipv6_mreq mreq6;
   memset(&mreq6.ipv6mr_multiaddr, 0, sizeof(struct in6_addr));
   memcpy(&mreq6.ipv6mr_multiaddr, addr, sizeof(struct in6_addr));
   mreq6.ipv6mr_interface = ifindex;
   if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) == -1) return 0;
   else return 1;
}

int join_groups(const int family, const int sockfd, const struct in6_addr *addrs, const int parac)
{
   if(family == AF_INET6)
   {
      //join the all nodes group in link scope
      if(join_agroup(sockfd, &allnodes_link) == 0) printf("Error %s when join the all nodes group\n", strerror(errno));

      //join the solicitation nodes group
      struct in6_addr solicitation_nodes_link = ifaddr;
      memcpy(&solicitation_nodes_link, ipv6_multicast_prefix, 13);
      if(join_agroup(sockfd, &solicitation_nodes_link) == 0) printf("Error %s when join the solicitation nodes group\n", strerror(errno));

      //join parameter addr
      for(int i = 0; i < parac; ++i)
      {
         if(join_agroup(sockfd, addrs + i) == 0)
            printf("Error %s when join the group%d\n", strerror(errno), i);
         continue;
      }
      return 1;
   }
}

int to_solicitation_node_groupaddr(const char *tar, struct in6_addr *res)
{
   if(inet_pton(AF_INET6, tar, (void *)res) != 1) return 0;
   memcpy(res->s6_addr, ipv6_multicast_prefix, 13);
   return 1;
}