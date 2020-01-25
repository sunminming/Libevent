#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

//callback function of interrupt event
void interrupt_cb(evutil_socket_t fd, short what, void *arg)
{
   struct event_base *base = (struct event_base *)arg;
   struct timeval delay = {1, 0};
   printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");
   event_base_loopexit(base, &delay);
}

//callback function of stdin event
void stdin_cb(evutil_socket_t fd, short what, void *arg)
{
   char buf[1024] = {0};
   struct bufferevent *bev = (struct bufferevent *)arg;
   read(fd, buf, sizeof(buf));
   bufferevent_write(bev, buf, strlen(buf)-1);
}


//create a signal event for interrupt and add into event base
void add_signal(struct event_base *base)
{
   struct event *signal_event;
   signal_event = evsignal_new(base, SIGINT, interrupt_cb, (void *)base);
   event_add(signal_event, NULL);
}

//create a write event of stdin and add into event base
void add_stdin(struct event_base *base, struct bufferevent *bev)
{
   struct event *ev = event_new(base, STDIN_FILENO,
                                 EV_READ | EV_PERSIST, stdin_cb, bev);
   event_add(ev, NULL);
}

