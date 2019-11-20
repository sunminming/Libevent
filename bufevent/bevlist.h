#ifndef BEVLIST
#define BEVLIST
//#include <malloc.h>
#include <event2/bufferevent.h>
struct bevnode
{
    struct bufferevent *bev;
    struct bevnode *next;
};

struct bevnode *bevnode_add(struct bevnode *, struct bufferevent *);

struct bevnode *bevnode_del(struct bevnode *, struct bufferevent *);

void foreach_send(struct bevnode *, char *);

#endif
