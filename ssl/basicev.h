#ifndef BASICEV
#define BASICEV
#include <event2/event.h>
#include <event2/bufferevent.h>

//create a signal event for interrupt and add into event base
void add_signal(struct event_base *);
//create a stdin event and add into event base
void add_stdin(struct evnet_base *, struct bufferevent *);

//callback function of interrupt event
void interrupt_cb(evutil_socket_fd, short, void *);
//callback function of stdin event
void stdin_cb(evutil_socket_fd, short, void *);
#endif
