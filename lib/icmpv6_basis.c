#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include "sock_basis.h"

struct nd_neighbor_solicit mynshdr_p(char *tar)
{
   struct nd_neighbor_solicit nshdr;
   nshdr.nd_ns_type = 135;
   nshdr.nd_ns_code = 0;
   struct in6_addr tar_addr, src_addr;
   inet_pton(AF_INET6, tar, (void *)&tar_addr);
   nshdr.nd_ns_target = tar_addr;

   return nshdr;
}

ssize_t send_ns(int sockfd, char *tar)
{
   struct in6_addr tar_addr;
   struct sockaddr_in6 groupaddr;

   if(to_solicitation_node_groupaddr(tar, &tar_addr) == 0) return 0;

   groupaddr.sin6_scope_id = 2;
   groupaddr.sin6_flowinfo = 0;
   groupaddr.sin6_port = 0;
   groupaddr.sin6_family = AF_INET6;
   groupaddr.sin6_addr = tar_addr;

   struct nd_neighbor_solicit nshdr = mynshdr_p(tar);

   return sendto(sockfd, (void *)&nshdr, sizeof(struct nd_neighbor_solicit), 0, (struct sockaddr *)&groupaddr, sizeof(struct sockaddr_in6));
}