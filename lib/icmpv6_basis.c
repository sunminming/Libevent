#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sock_basis.h"

#define MAX_MSGLEN 1024

extern int ifindex;
extern struct in6_addr allnodes_link;
extern struct ether_addr macaddr;

int mynshdr_p(char *tar, char *buf)
{
   //basic ns header
   struct nd_neighbor_solicit *nshdr = (struct nd_neighbor_solicit *)buf;
   nshdr->nd_ns_type = 135;
   nshdr->nd_ns_code = 0;
   if(inet_pton(AF_INET6, tar, (void *)&(nshdr->nd_ns_target)) != 1) return 0;

   //nd option header
   struct nd_opt_hdr *opthdr = (struct nd_opt_hdr *)(buf + sizeof(struct nd_neighbor_solicit));
   opthdr->nd_opt_len = 1;
   opthdr->nd_opt_type = ND_OPT_SOURCE_LINKADDR;

   //src mac
   memcpy(buf + sizeof(struct nd_neighbor_solicit) + sizeof(struct nd_opt_hdr), &macaddr, sizeof(struct ether_addr));
   return 1;
}

ssize_t send_ns(int sockfd, char *tar)
{
   ssize_t nslen = sizeof(struct nd_neighbor_solicit) + sizeof(struct nd_opt_hdr) + sizeof(struct ether_addr);
   struct in6_addr tar_addr;
   struct sockaddr_in6 groupaddr;

   if(to_solicitation_node_groupaddr(tar, &tar_addr) == 0) return 0;

   groupaddr.sin6_scope_id = ifindex;
   groupaddr.sin6_flowinfo = 0;
   groupaddr.sin6_port = 0;
   groupaddr.sin6_family = AF_INET6;
   groupaddr.sin6_addr = tar_addr;

   char *nsdata = (char *)malloc(nslen);
   memset(nsdata, 0, nslen);

   if(mynshdr_p(tar, nsdata) != 1) return 0;
   else
   {
      int res;
      for(int i = 0; i < 5; ++i)
      {
         res = sendto(sockfd, (void *)nsdata, nslen, 0, (struct sockaddr *)&groupaddr, sizeof(struct sockaddr_in6));
         if(res > 0) return res;
         else
         {
            sleep(0.5);
            continue;
         }
      }
      return res;
   }
}

int prov_ns(const char *nsbuf, const ssize_t nslen)
{
   if(nslen < sizeof(struct nd_neighbor_solicit)) return 0;
   struct nd_neighbor_solicit *nshdr = (struct nd_neighbor_solicit *)nsbuf;

   char *tar = (char *)malloc(INET6_ADDRSTRLEN);
   memset(tar, '\0', INET6_ADDRSTRLEN);

   if(inet_ntop(AF_INET6, (void *)&(nshdr->nd_ns_target), tar, INET6_ADDRSTRLEN) != NULL)
   {
      printf("target address %s\n", tar);
      return 1;
   }
   else return 0;
}

void recv_ns(const int sockfd)
{
   //pass na and block other icmp6 packet
   struct icmp6_filter nafilter;
   ICMP6_FILTER_SETBLOCKALL(&nafilter);
   ICMP6_FILTER_SETPASS(ND_NEIGHBOR_SOLICIT, &nafilter);
   
   //buffer for received data
   char *nsbuf = (char *)malloc(MAX_MSGLEN);
   memset(nsbuf, '\0', MAX_MSGLEN);

   //the srouce address
   struct sockaddr_in6 *ns_sender = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
   memset(ns_sender, 0, sizeof(struct sockaddr_in6));
   socklen_t addrlen = sizeof(struct sockaddr_in6);

   ssize_t nslen = recvfrom(sockfd, nsbuf, MAX_MSGLEN, 0, (struct sockaddr *)ns_sender, &addrlen);

   if(prov_ns(nsbuf, nslen) == 0)
   {
      printf("Error when process NS packet");
      return;
   } 
   free(nsbuf);
   free(ns_sender);
}