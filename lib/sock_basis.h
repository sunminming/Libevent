#ifndef SOCKBASIS
#define SOCKBASIS

#include <netinet/in.h>

/*
Function init_sock is used to initialize a socket, only supports ICMPv6 for the time being
int family: a protocol family type constant, as defined in sys/socket.h;
int protocol: a protocol type constant, as defined in sys/socket.h;
void *parameter: some parameter for initialization, such as bind()/connect() for TCP;
return the socket, or -1;
*/
int init_sock(int family, int protocol, void *parameter);

/*
Function ipaddr is used to initialize a sockaddr struct, only supports IPv6 for the time being
int family: a protocol family type constant, as defined in sys/socket.h;
char *addr: a address string, such as "::1" or "10.0.0.1";
int scope_id: the scope id for sockaddr_in6 struct;
void *res: a address for save the result, sockadd_in for ipv4 ot sockaddr_in6 for ipv6;
return the length of the result;
*/
socklen_t ipaddr(int family, char *addr, int scope_id, void *res);

/*
Function join_groupaddr6 is used to join a socket into a multicast group
int sockfd: the fd;
char *addr: a address string to indicate the multicasr address;
int interface: the interface id;
return 1 if success, or 0;
*/
int join_groupaddr6(int sockfd, char *addr, int interface);

/*
Function group_join is used to join a socket into a set of multicast groups, such as "ff01::2" and other groups specified by parameter
int family: a protocol family type constant, as defined in sys/socket.h;
int sockfd: the fd;
char **parameter: the set of groups;
int parac: the length of parameter;
return 1 if success, or 0;
*/
int group_join(int family, int sockfd, char **parameter, int parac);

/*
Function to_solicitation_node_groupaddr is used to transfer a ipv6 unicast address to solicitation node multicast address
char *tar: the ipv6 unicast address;
struct in6_addr *res: the multicast address result;
return 1 if success, or 0;
*/
int to_solicitation_node_groupaddr(char *tar, struct in6_addr *res);
#endif