#ifndef ICMPV6BASIS
#define ICMPV6BASIS

#include <netinet/icmp6.h>
/*
Function mynshdr_p is used to construct a neighbor solicitation header with a string
char *tar: the target ipv6 address in this NS;
return the NS header;
*/
struct nd_neighbor_solicit mynshdr_p(char *tar);

/*
Function send_NS is used to send a neighbor solicit header for solicit a neighbor mac
int sockfd: the fd;
char *tar: the target ipv6 address in this NS;
return the length of NS packet sent;
*/
ssize_t send_ns(int sockfd, char *tar);

#endif