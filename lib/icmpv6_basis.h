#ifndef ICMPV6BASIS
#define ICMPV6BASIS

#include <netinet/icmp6.h>
/*
Function mynshdr_p is used to construct a neighbor solicitation header with a string
char *tar: the target ipv6 address in this NS;
struct nd_neighbor_solicit *res: the NS packet result;
return 1 if success, or 0;
*/
int mynshdr_p(const char *tar, char *res);

/*
Function send_ns is used to send a neighbor solicit packet for solicit a neighbor mac
int sockfd: the fd;
char *tar: the target ipv6 address in this NS;
return the length of NS packet sent;
*/
ssize_t send_ns(int sockfd, char *tar);

/*
Function prov_ns is used to process a neighbor solicitation packet
const char *nsbuf: the received data;
const ssize_t nslen: the length of received data;
return 1 if success, or 0;
*/
int prov_ns(const char *nsbuf, const ssize_t nslen);

/*
Function recv_ns is used to recv a neighbor solicitation packet for required mac
int sockfd: the fd;
char *tar: the target ipv6 address in this NS;
*/
void recv_ns(const int sockfd);

#endif