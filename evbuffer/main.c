#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/util.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <event2/listener.h>
#include <errno.h>
#include <signal.h>

static void conn_eventcb(struct bufferevent *bev, short events, void *ptr)
{
   if(events & BEV_EVENT_EOF)
   {
      printf("client has closed the connection!\n");
   }
   if(events & BEV_EVENT_ERROR)
   {
      printf("got an error on the connection: %s\n", strerror(errno));
   }
   bufferevent_free(bev);
}

static void evbuffer_cb(struct evbuffer *buffer, const struct evbuffer_cb_info *info, void *ptr)
{
   if(info->n_added > 1)
   {
      printf("The output buffer has added %ld bytes\n", info->n_added);
      //char *data[10] = {0};
      //evbuffer_clear_flags(buffer, EVBUFFER_FLAG_DRAINS_TO_FD);
      evbuffer_unfreeze(buffer, 1);
      
      evbuffer_clear_flags(buffer, EVBUFFER_FLAG_DRAINS_TO_FD);
      evbuffer_drain(buffer, 1);
            
      evbuffer_unfreeze(buffer, 1);
   }
   if(info->n_deleted > 0)
   {
      printf("The output buffer has deleted %ld bytes\n", info->n_deleted);
   }

}

static void evbuffer_set(struct bufferevent *bev)
{
   struct evbuffer *output = bufferevent_get_output(bev);
   struct evbuffer_cb_entry *evbuffer_callback = evbuffer_add_cb(output, evbuffer_cb, NULL);
   //struct evbuffer_cb_entry* del_callback =  evbuffer_add_cb(output, evbuffer_del_cb, NULL);
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
		struct sockaddr *sa, int socklen, void *ptr)
{
   struct event_base *base = ptr;
   struct bufferevent *bev = NULL;

   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
   bufferevent_setcb(bev, NULL, NULL, conn_eventcb, NULL);

   bufferevent_enable(bev, EV_WRITE);
   bufferevent_disable(bev, EV_READ);
   
   evbuffer_set(bev);
   int file_segment = open("main.c", O_RDONLY);
   struct evbuffer_file_segment *file_seg = evbuffer_file_segment_new(file_segment, 0, -1, EVBUF_FS_CLOSE_ON_FREE);
   struct evbuffer *output = bufferevent_get_output(bev);
   if(evbuffer_add_file_segment(output, file_seg, 0, 10) == -1)
   {
      printf(strerror(errno));
   }

   //bufferevent_write(bev, "Hello, this is a test!", strlen("Hello, this is a test!"));
}

static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
        struct event_base *base = user_data;
        struct timeval delay = { 2, 0 };

        printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

        event_base_loopexit(base, &delay);
}

int main(int argc, char *argv[])
{
   struct event_base *base = NULL;
   base = event_base_new();

   struct sockaddr_in sin;
   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons(9995);
   
   struct evconnlistener *listener;
   listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
		   LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr *)&sin, sizeof(sin));

   struct event *signal_ev;
   signal_ev = evsignal_new(base, SIGINT, signal_cb, (void *)base);
   
   event_add(signal_ev, NULL);
   event_base_dispatch(base);

   evconnlistener_free(listener);
   event_free(signal_ev);
   event_base_free(base);
   
   return 0;
}

