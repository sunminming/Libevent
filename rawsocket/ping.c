#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <error.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

int nsent = 0;//seq of icmp echo

uint16_t in_cksum(uint16_t *addr, int len)
{
   int nleft = len;
   uint32_t sum = 0;
   uint16_t *w = addr;
   uint16_t answer = 0;

   while(nleft > 1)
   {
      sum += *w++;
      nleft -= 2; 
   }

   if(nleft == 1)
   {
      *(unsigned char *)(&answer) = *(unsigned char *)w; 
      sum += answer;
   }

   sum = (sum >> 16) + (sum & 0xffff);
   sum += (sum >> 16);
   answer = ~sum;
   return answer;
}

void sendicmp(int rawsockfd, struct sockaddr_in *pairaddr)
{
   struct icmp *icmphead;
   int len = 8 + 56;
   char buf[1024];
   icmphead = (struct icmp *)buf;
   icmphead->icmp_type = ICMP_ECHO;
   icmphead->icmp_code = 0;
   icmphead->icmp_id = getpid();
   icmphead->icmp_seq = htons((uint16_t)2);
   memset(icmphead->icmp_data, 0, 56);
   icmphead->icmp_cksum = 0;
   icmphead->icmp_cksum = in_cksum((uint16_t *)icmphead, len);
   sendto(rawsockfd, icmphead, len, 0, (struct sockaddr *)pairaddr, sizeof(struct sockaddr));
}

int main(int argc, char **argv)
{
   int rawsockfd;// the raw socket
   struct sockaddr_in *pairaddr;//pair address

   if(argc != 2)
   {
      printf("Please input pair address\n");
      exit(1); 
   }
   //init pair address
   pairaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
   memset(pairaddr, 0, sizeof(struct sockaddr_in));
   pairaddr->sin_family = AF_INET;
   inet_pton(AF_INET, argv[1], &(pairaddr->sin_addr.s_addr));
   //inti raw socket
   if((rawsockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
   {
      perror("A ERROR IN socket()");
      exit(1); 
   }
   
   sendicmp(rawsockfd, pairaddr);
   return 0;
}