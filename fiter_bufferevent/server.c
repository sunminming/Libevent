#include <event2/bufferevent.h>
#include <event2/event.h>
#include <arpa/inet.h>// interner address
#include <unistd.h>//os
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <event2/listener.h>
#include <sys/types.h>
#include <errno.h>

typedef enum bufferevent_filter_result input_cb(struct evbuffer *src, struct evbuffer *dst, 
		ev_ssize_t dst_limit, enum bufferevent_flush_mode mode, void *ctx)
{
   
}


void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
		 struct sockaddr *addr, int len, void *ptr)
{
   struct sockaddr_in *caddr = (struct sockaddr_in *)addr;
   struct event_base *base = (struct event_base *)ptr;
   
   //init bufferevent
   struct bufferevent *bev;
   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
   
   bufferevent_setcb(bev, NULL, NULL, bev_event_cb, NULL);
   bufferevent_enable(bev, EV_WRITE | EV_READ);
   buferevent_write(bev, "Hello client!", strlen("Hello client!")+1);
   
   struct filter_bev = bufferevent_filter_new(bev, input_cb, output_cb, BEC_OPT_CLOSE_ON_FREE, 
		   NULL, NULL);
}


int main(int argc, const char *argv[])
{
   //init server
   struct sockaddr_in servaddr;
   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(9876);
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

   //init event_base
   struct event_base *base;
   base = event_base_new();

   //init linstener
   struct evconnlistener *listener;
   listener = evconnlistener_new_bin(base, linstener_cb, base, 
		   LEV_OPT_CLOSE_ON_FREE, 36, 
		   (struct socketaddr *)&servaddr, sizeof(servaddr));
   event_base_dispath(base);

   evconnlistener_free(listener);
   event_base_free(base);

}
