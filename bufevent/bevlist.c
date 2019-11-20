#include <malloc.h>
#include <string.h>
#include "bevlist.h"
#include <event2/bufferevent.h>
struct bevnode *bevnode_add(struct bevnode* head, struct bufferevent *bev)
{
    struct bevnode *new_bevn = (struct bevnode *)malloc(sizeof(struct bevnode));
    new_bevn->bev = bev;
    new_bevn->next = head;
    return new_bevn;
}

struct bevnode *bevnode_del(struct bevnode *head, struct bufferevent *target_bev)
{
    if(head == NULL) return head;
    struct bevnode *q = head;
    if(q->bev == target_bev)
    {
        return head->next;
    }
    struct bevnode *p = q->next;
    while(p)
    {
        if(p->bev == target_bev)
	{
	    q->next = p->next;
	    return head;
	}
	else
	{
	    q = q->next;
	    p = p->next;
	}
    }
    return head;
}

void foreach_send(struct bevnode *head, char *message)
{
    struct bevnode *p = head;
    while(p)
    {
        bufferevent_write(p->bev, message, strlen(message)+1);
	p = p->next;
    }
}


