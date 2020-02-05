#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

int rawsockfd;// the raw socket
struct sockaddr_in *pairaddr, recvaddr;//pair address
int nsent = 0;//seq of icmp echo
int interval;

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
   icmphead->icmp_seq = htons((uint16_t)nsent++);
   memset(icmphead->icmp_data, 0, 56);
   icmphead->icmp_cksum = 0;
   icmphead->icmp_cksum = in_cksum((uint16_t *)icmphead, len);
   sendto(rawsockfd, icmphead, len, 0, (struct sockaddr *)pairaddr, sizeof(struct sockaddr));
}

void fun_sigalrm(int sigalrm)
{
   sendicmp(rawsockfd, pairaddr);
   alarm(interval);
}

void fun_recv(char *buf, ssize_t n)
{
   struct ip *iphead;
   struct icmp *icmphead;
   int iphdrlen;
   //char *srcipaddr = malloc(16);
   //char *dstipaddr = malloc(16);
   //struct sockaddr_in *addr = msg->msg_name;
   iphead = (struct ip *)buf;
   if(iphead->ip_p != IPPROTO_ICMP)
   {
      printf("reciev a ip which is not icmp\n");
      return;
   }
   iphdrlen = (iphead->ip_hl) << 2;
   icmphead = (struct icmp *)(buf + iphdrlen);//(iphead + iphdrlen) is wrong because it will add iphdrlen*sizeof(iphead)
   int seq = ntohs(icmphead->icmp_seq);
   
   if(icmphead->icmp_type == ICMP_ECHOREPLY && icmphead->icmp_id == getpid())
   {
     printf("receive seq %d\n", seq); 
   }
   else
   {
      if(icmphead->icmp_id != getpid())
      {
         printf("receive a error echoreply from %d\n", icmphead->icmp_id);
      }
   }
   
   return;
}

int main(int argc, char **argv)
{
   struct sigaction act;
   struct msghdr msg;
   struct iovec iov;

   if(argc != 3)
   {
      printf("Please input pair address and interval\n");
      exit(1); 
   }
   interval = atoi(argv[2]);
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
   //init sigaction
   act.sa_handler = fun_sigalrm;
   sigemptyset(&act.sa_mask);
   act.sa_flags = SA_RESTART;
   sigaction(SIGALRM, &act, NULL);
   alarm(interval);
   //init msg for recv
   memset(&msg, 0, sizeof(msg));
   memset(&recvaddr, 0, sizeof(recvaddr));
   msg.msg_name = &recvaddr;
   msg.msg_namelen = sizeof(recvaddr);
   char *buf=malloc(1024);
   memset(buf, 0, 1024);
   iov.iov_base = buf;
   iov.iov_len = 1024;
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
   
   while(1)
   {
      ssize_t n = recvmsg(rawsockfd, &msg, 0);
      fun_recv(buf, n);
   }
   return 0;
}