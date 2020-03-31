#ifndef SOCKBASIS
#define SOCKBASIS

#include <netinet/in.h>
#include <net/ethernet.h>
#include <netdb.h>
/*
Function init_sock is used to initialize a socket, only supports ICMPv6 for the time being
int family: a protocol family type constant, as defined in sys/socket.h;
int protocol: a protocol type constant, as defined in sys/socket.h;
void *parameter: some parameter for initialization, such as bind()/connect() for TCP;
return the socket, or -1;
*/
int init_sock(const int family, const int protocol, void *parameter);

/*
Function addr_fromname is used to get the address of interface
int family: a protocol family type constant, as defined in sys/socket.h;
char *name: the interface name;
void *res: the multicast address result;
int *len: a value-result parament, it indicates the size of res when input and return the size of result;
return 1 if success, or 0;
*/
int addr_fromname(const int family, const char *name, void *res, int *len);

/*
Function mac_fromname is used to get the mac address
char *name: the interface name;
void *res: the mac address result;
return 1 if success, or 0;
*/
int mac_fromname(const char *name, struct ether_addr *res);

/*
Function join_groupaddr6 is used to join a socket into a multicast group
int sockfd: the fd;
char *addr: a address string to indicate the multicasr address;
int ifindex: the interface index for sockaddr_in6.scope_id;
return 1 if success, or 0;
*/
int join_agroup(const int sockfd, const struct in6_addr *addr);

/*
Function group_join is used to join a socket into a set of multicast groups, such as "ff01::2" and other groups specified by parameter
int family: a protocol family type constant, as defined in sys/socket.h;
int sockfd: the fd;
struct in6_addr *addrs: the set of group address;
int parac: the length of parameter;
return 1 if success, or 0;
*/
int join_groups(const int family, const int sockfd, const struct in6_addr *addrs, const int parac);

/*
Function to_solicitation_node_groupaddr is used to transfer a ipv6 unicast address to solicitation node multicast address
char *tar: the ipv6 unicast address;
struct in6_addr *res: the multicast address result;
return 1 if success, or 0;
*/
int to_solicitation_node_groupaddr(const char *tar, struct in6_addr *res);

/*
Function free_allai is used to free the memory of struct addrinfo
struct addrinfo *res: the point to memory of struct addrinfo
*/
void free_allai(struct addrinfo *res);

#endif