#ifndef ICMPV6BASIS
#define ICMPV6BASIS

#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <pthread.h>

//node for saving ns packet recived
struct recvns_node
{
   struct in6_addr solicitation_tar;
   struct in6_addr solicitation_src;
   struct recvns_node *next;
};

//link of struct recvns_node
struct recvns_data
{
   struct recvns_node *head;
   unsigned int length;
   pthread_mutex_t lock;
};

//parament of function 
struct arg_recv_ns
{
   struct recvns_data *ns_head;
   int sockfd;
};

/*
Function init_recvnslink is init a head of recvns_data link
return a point of struct recvns_data;
*/
struct recvns_data *init_recvnslink();

/*
Function init_recvnslink is free a memory of recvns_data link
struct recvns_data *ns_head: the point of struct recvns_data
*/
void free_recvnslink(struct recvns_data *ns_head);

/*
Function insert_nsnode is used to insert a node into the head of recvns_data link
struct recvns_data *ns_head: the point of struct recvns_data;
*/
void insert_nsnode(struct recvns_data *ns_head);

/*
Function delete_nsnode is used to delete the last node in recvns_data link
struct recvns_data *ns_head: the point of struct recvns_data;
*/
void delete_nsnode(struct recvns_data *ns_head);

/*
Function nshdr_create is used to construct a neighbor solicitation header with a string
char *tar: the target ipv6 address in this NS;
struct nd_neighbor_solicit *res: the NS packet result;
return 1 if success, or 0;
*/

int nshdr_create(const char *tar, char *res);

/*
Function send_ns is used to send a neighbor solicitation packet for solicit a neighbor mac
int sockfd: the fd;
char *tar: the target ipv6 address in this NS;
return the length of NS packet sent;
*/
ssize_t send_ns(int sockfd, char *tar);

/*
Function prov_ns is used to process a neighbor solicitation packet
const char *nsbuf: the received data;
const ssize_t nslen: the length of received data;
struct in6_addr *src: the source of NS;
return 1 if success, or 0;
*/
int record_ns(char *nsbuf, const ssize_t nslen, struct recvns_data *ns_head, struct in6_addr *src);

/*
Function recv_ns is used to recv a neighbor solicitation packet
void *arg: a point to struct arg_recv_ns;
char *tar: the target ipv6 address in this NS;
*/
void *recv_ns(void *arg);

/*
Function listen_ns is used to listen a neighbor solicitation packet
int sockfd: the fd;
int flag: a flag to indicate whether send na;
return a point of struct recvns_data;
*/
struct recvns_data *listen_ns(pthread_t *tid);

/*
Function print_ns is used to print the NS link
struct recvns_data *nsdata: a point of struct recvns_data;
*/
void print_ns(struct recvns_data *nsdata);

/*
Function send_na is used to send a na message for a recived NS
struct in6_addr *dest: a point of struct in6_addr to indicate the NS sender, i.e., the NA receiver;
return the length of NA packet sent;
*/
ssize_t send_na(const int sockfd, struct in6_addr *dest, char *nsbuf, const ssize_t nslen);

/*
Function reqhdr_create is used to construct a echo request header with target address
struct in6_addr *dest: the dest ipv6 address in this NS;
struct icmp6_hdr reqhdr: the returned header;
*/
void reqhdr_create(struct in6_addr *dest, struct icmp6_hdr *reqhdr);

/*
Function reqdataset is used to construct the data field of echo request header
char *reqdata: a point to the data field of echo request header;
*/
void reqdataset(char *reqdata);
/*
Function ping is used to send a echo request to dest host
int sockfd: the fd;
char *tar: the dest ipv6 address in this NS;
return the length of NA packet sent;
*/
ssize_t ping(const int sockfd, const char *dest);
#endif