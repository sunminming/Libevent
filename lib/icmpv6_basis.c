#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sock_basis.h"
#include "icmpv6_basis.h"

#define MAX_MSGLEN 1024
#define MAX_NSLEN 10

uint16_t echo_seq = 1;
extern int ifindex;
extern int naflag;
extern struct in6_addr allnodes_link;
extern struct in6_addr ifaddr;
extern struct ether_addr macaddr;

struct recvns_data *init_recvnslink()
{
   struct recvns_data *ns_head = (struct recvns_data *)malloc(sizeof(struct recvns_data));
   ns_head->head = NULL;
   ns_head->length = 0;
   pthread_mutex_init(&(ns_head->lock), NULL);
   return ns_head;
}

void free_recvnslink(struct recvns_data *ns_head)
{
   struct recvns_node *temp = ns_head->head;
   struct recvns_node *t;
   while(temp)
   {
      t = temp->next;
      free(temp);
      temp = t;
   }
   pthread_mutex_destroy(&(ns_head->lock));
   free(ns_head);
}

void insert_nsnode(struct recvns_data *ns_head)
{
   struct recvns_node *node = (struct recvns_node *)malloc(sizeof(struct recvns_node));
   node->next = ns_head->head;
   ns_head->head = node;
   ++(ns_head->length);
   return;
}

void delete_nsnode(struct recvns_data *ns_head)
{
   struct recvns_node *node = ns_head->head;
   while(node->next) node = node->next;
   --(ns_head->length);
   return;
}

int nshdr_create(const char *tar, char *res)
{
   //basic ns header
   struct nd_neighbor_solicit *nshdr = (struct nd_neighbor_solicit *)res;
   nshdr->nd_ns_type = 135;
   nshdr->nd_ns_code = 0;
   if(inet_pton(AF_INET6, tar, (void *)&(nshdr->nd_ns_target)) != 1) return 0;

   //nd option header
   struct nd_opt_hdr *opthdr = (struct nd_opt_hdr *)(res + sizeof(struct nd_neighbor_solicit));
   opthdr->nd_opt_len = 1;
   opthdr->nd_opt_type = ND_OPT_SOURCE_LINKADDR;

   //src mac
   memcpy(res + sizeof(struct nd_neighbor_solicit) + sizeof(struct nd_opt_hdr), &macaddr, sizeof(struct ether_addr));
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

   if(nshdr_create(tar, nsdata) != 1)
   {
      free(nsdata);
      return 0;
   }
   else
   {
      int res = 0;
      for(int i = 0; i < 5; ++i)
      {
         res = sendto(sockfd, (void *)nsdata, nslen, 0, (struct sockaddr *)&groupaddr, sizeof(struct sockaddr_in6));
         if(res > 0) break;
         else
         {
            if(errno == EINTR) sleep(0.5);
            else break;
         }
      }
      free(nsdata);
      return res;
   }
}

int record_ns(char *nsbuf, const ssize_t nslen, struct recvns_data *ns_head, struct in6_addr *src)
{
   if(nslen < sizeof(struct nd_neighbor_solicit)) return 0;
   struct nd_neighbor_solicit *nshdr = (struct nd_neighbor_solicit *)nsbuf;

   if(ns_head->length < MAX_NSLEN)
   {
      pthread_mutex_lock(&(ns_head->lock));
      insert_nsnode(ns_head);
      ns_head->head->solicitation_tar = nshdr->nd_ns_target;
      ns_head->head->solicitation_src = *src;
      pthread_mutex_unlock(&(ns_head->lock));
   }
   else
   {
      pthread_mutex_lock(&(ns_head->lock));
      delete_nsnode(ns_head);
      insert_nsnode(ns_head);
      ns_head->head->solicitation_tar = nshdr->nd_ns_target;
      ns_head->head->solicitation_src = *src;
      pthread_mutex_unlock(&(ns_head->lock));
   }
   return 1;
}

void *recv_ns(void *arg)
{
   struct arg_recv_ns *args = (struct arg_recv_ns *)arg;
   const int sockfd = args->sockfd;
   int res = 1;
   struct recvns_data *ns_head = args->ns_head; 

   //buffer for received data
   char *nsbuf = (char *)malloc(MAX_MSGLEN);
   //the srouce address
   struct sockaddr_in6 *ns_sender = (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
   socklen_t addrlen = sizeof(struct sockaddr_in6);

   while(1)
   {
      memset(nsbuf, '\0', MAX_MSGLEN);
      memset(ns_sender, 0, sizeof(struct sockaddr_in6));
      ssize_t nslen = recvfrom(sockfd, nsbuf, MAX_MSGLEN, 0, (struct sockaddr *)ns_sender, &addrlen);
      if(nslen == -1) perror("Recvfrom");
      //else printf("Recv a NS;\n");
      if(record_ns(nsbuf, nslen, ns_head, &(ns_sender->sin6_addr)) == 0) perror("Process NS packet");
      else 
      {
         if(naflag == 1 && send_na(sockfd, &(ns_sender->sin6_addr), nsbuf, nslen) != nslen)
            perror("Send NA");
      }
   }
   free(ns_sender);
   free(nsbuf);
   return &res;
}

struct recvns_data * listen_ns(pthread_t *tid)
{
   int sockfd = init_sock(AF_INET6, IPPROTO_ICMPV6, NULL);
   if(sockfd == -1)
   {
      perror("In init_sock()");
      return NULL;
   }
   //pass icmpv6 ns message
   struct icmp6_filter *nafilter = (struct icmp6_filter *)malloc(sizeof(struct icmp6_filter));
   socklen_t filterlen = sizeof(struct icmp6_filter);
   ICMP6_FILTER_SETBLOCKALL(nafilter);
   ICMP6_FILTER_SETPASS(ND_NEIGHBOR_SOLICIT, nafilter);
   setsockopt(sockfd, IPPROTO_ICMPV6, ICMP6_FILTER, nafilter, sizeof(struct icmp6_filter));
   free(nafilter);

   struct arg_recv_ns *arg = (struct arg_recv_ns *)malloc(sizeof(struct arg_recv_ns));
   arg->ns_head = init_recvnslink();
   arg->sockfd = sockfd;
   if(pthread_create(tid, NULL, recv_ns, arg) == 0) return arg->ns_head;
   else
   {
      free_recvnslink(arg->ns_head);
      return NULL;
   }
}

void print_ns(struct recvns_data *nsdata)
{
   struct recvns_node *node = nsdata->head;
   int index = 1;
   char tar[INET6_ADDRSTRLEN], src[INET6_ADDRSTRLEN];
   if(node == NULL) printf("NONE;\n");
   while(node != NULL)
   {
      memset(tar, '\0', INET6_ADDRSTRLEN);
      memset(src, '\0', INET6_ADDRSTRLEN);
      if(inet_ntop(AF_INET6, &(node->solicitation_src), src, INET6_ADDRSTRLEN) &&
         inet_ntop(AF_INET6, &(node->solicitation_tar), tar, INET6_ADDRSTRLEN))
         printf("%d: NS target %s from host %s\n", index, tar, src);
      else printf("%d: Parse Error\n", index);
      ++index;
      node = node->next;
   }
   return;
}

ssize_t send_na(const int sockfd, struct in6_addr *dest, char *nsbuf, const ssize_t nslen)
{
   char *nabuf = nsbuf;
   struct nd_neighbor_advert *nahdr = (struct nd_neighbor_advert *)nabuf;
   if(memcmp(&(nahdr->nd_na_target), &ifaddr, sizeof(struct in6_addr)) != 0) return nslen;

   nahdr->nd_na_hdr.icmp6_type = ND_NEIGHBOR_ADVERT;
   nahdr->nd_na_flags_reserved = ND_NA_FLAG_SOLICITED;
   
   struct nd_opt_hdr *opthdr = (struct nd_opt_hdr *)(nabuf + sizeof(struct nd_neighbor_advert));
   opthdr->nd_opt_type = ND_OPT_TARGET_LINKADDR;

   char *optdata = nabuf + sizeof(struct nd_neighbor_advert) + sizeof(struct nd_opt_hdr);
   memcpy(optdata, &macaddr, sizeof(struct ether_addr));

   struct sockaddr_in6 nadest;
   nadest.sin6_scope_id = ifindex;
   nadest.sin6_flowinfo = 0;
   nadest.sin6_port = 0;
   nadest.sin6_family = AF_INET6;
   nadest.sin6_addr = *dest;
   
   ssize_t len = sendto(sockfd, nabuf, nslen, 0, (const struct sockaddr *)&nadest, sizeof(struct sockaddr_in6));
   //printf("Send a NA with %d bytes\n", len);
   return len;
}

void reqdataset(char *reqdata)
{
   return;
}

void reqhdr_create(struct in6_addr *dest, struct icmp6_hdr *reqhdr)
{
   reqhdr->icmp6_code = 0;
   reqhdr->icmp6_type = ICMP6_ECHO_REQUEST;
   memcpy(&(reqhdr->icmp6_id), &macaddr, 16);
   reqhdr->icmp6_seq = htons(echo_seq++);
   reqdataset((char *)(reqhdr + 1));
   return;
}

ssize_t ping(const int sockfd, const char *dest)
{
   struct sockaddr_in6 destaddr;
   ssize_t res = 0;
   ssize_t single_res = 0;
   int cnt = 0;
   if(inet_pton(AF_INET6, dest, (void *)&(destaddr.sin6_addr)) != 1) return 0;
   destaddr.sin6_family = AF_INET6;
   destaddr.sin6_flowinfo = 0;
   destaddr.sin6_port = 0;
   destaddr.sin6_scope_id = ifindex;

   ssize_t reqlen = sizeof(struct icmp6_hdr) + 56 * sizeof(uint8_t);
   char reqhdr[reqlen];
   memset(&reqhdr, 0, sizeof(struct icmp6_hdr));
   reqhdr_create(&(destaddr.sin6_addr), (struct icmp6_hdr *)reqhdr);
   
   while(cnt++ < 5)
   {
      single_res += sendto(sockfd, reqhdr, reqlen, 0, (struct sockaddr *)&destaddr, sizeof(struct sockaddr_in6));
      res += single_res;
      printf("Send a Echo Request with %ld bytes\n", single_res);
   }
   printf("Send %d requests in total with %ld bytes", cnt, res);
}