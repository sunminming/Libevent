#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <arpa/inet.h>// interner address
#include <unistd.h>//os
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <event2/listener.h>
#include <sys/types.h>
#include <errno.h>

enum bufferevent_filter_result input_cb(struct evbuffer *src, struct evbuffer *dst, 
		ev_ssize_t dst_limit, enum bufferevent_flush_mode mode, void *ctx)
{
   //printf("the src and dst in fun input_cb: %ld, %ld\n", src, dst);
   //evbuffer_drain(src, 1024);
   char buf[1024];
   memset(buf, '\0', sizeof(buf));
   evbuffer_remove(src, buf, sizeof(buf));
   int len = strlen(buf);
   printf("%d", len);
   for(int i = 0; i < len; ++i)
   {
      ++buf[i];
   }
   buf[2*len] = '\n';
   evbuffer_add(dst, buf, len);
   return BEV_OK;   
}

enum bufferevent_filter_result output_cb(struct evbuffer *src, struct evbuffer *dst,
                ev_ssize_t dst_limit, enum bufferevent_flush_mode mode, void *ctx)
{
   //printf("!");
   char buf[1024] = {0};
   memset(buf, '\0', sizeof(buf));
   evbuffer_remove(src, buf, sizeof(buf));
   int len = strlen(buf);
   for(int i = len; i < 2*len; ++i)
   {
      buf[i] = buf[i - len];
   }
   evbuffer_add(dst, buf, 2*len);
   return BEV_OK;
}


void read_cb(struct bufferevent *bev, void *arg)
{
   char buf[1024] = {0};
   bufferevent_read(bev, buf, 1024);
   printf("%s\n", buf);
}

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
		 struct sockaddr *addr, int len, void *ptr)
{
   struct sockaddr_in *caddr = (struct sockaddr_in *)addr;
   struct event_base *base = (struct event_base *)ptr;
   
   //init bufferevent
   struct bufferevent *bev;
   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
   
   bufferevent_setcb(bev, NULL, NULL, NULL, NULL);
   bufferevent_enable(bev, EV_WRITE | EV_READ);
   
   struct bufferevent *filter_bev = bufferevent_filter_new(bev, input_cb, output_cb, BEV_OPT_CLOSE_ON_FREE, NULL, NULL);
   bufferevent_setcb(filter_bev, read_cb, NULL, NULL, NULL);
   bufferevent_enable(filter_bev, EV_READ|EV_WRITE);

   bufferevent_write(filter_bev, "abc", sizeof("abc"));
}


int main(int argc, const char *argv[])
{
   //init server
   struct sockaddr_in servaddr;
   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(9995);
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

   //init event_base
   struct event_base *base;
   base = event_base_new();

   //init linstener
   struct evconnlistener *listener;
   listener = evconnlistener_new_bind(base, listener_cb, base, 
		   LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE_PORT, 36, 
		   (struct socketaddr *)&servaddr, sizeof(servaddr));
   event_base_dispatch(base);

   evconnlistener_free(listener);
   event_base_free(base);

}
