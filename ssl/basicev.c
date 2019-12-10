#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

//callback function of interrupt event
void interrupt_cb(evutil_socket_fd fd, short what, void *arg)
{
   struct event_base *base = (struct event_base *)arg;
   struct timeval delay = {1, 0};
   printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");
   event_base_loopexit(base, &delay);
}
//callback function of stdin event
void stdin_cb(evutil_socket_fd fd, short what, void *arg)
{
   char buf[1024] = {0};
   struct bufferevent *bev = (struct bufferevent *)arg;
   read(fd, buf, sizeof(buf));
   bufferevent_write(bev, buf, strlen(buf));
}


//create a signal event for interrupt and add into event base
void add_signal(struct event_base *base)
{
   
}
//create a write event of stdin and add into event base
void add_stdin(struct evnet_base *, struct bufferevent *);

