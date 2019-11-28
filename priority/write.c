#include <event2/event.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void write_cb0(evutil_socket_t fd, short what, void *arg)
{
   char buf[1024] = {0};
   sprintf(buf, "%s", "This is 0\n");
   write(fd, buf, strlen(buf)+1);
}

void write_cb1(evutil_socket_t fd, short what, void *arg)
{
   char buf[1024] = {0};
   sprintf(buf, "%s", "This is 1\n");
   write(fd, buf, strlen(buf)+1);
}

void write_cb2(evutil_socket_t fd, short what, void *arg)
{
   char buf[1024] = {0};
   sprintf(buf, "%s", "This is 2\n");
   write(fd, buf, strlen(buf)+1);
}


int main(int argc, const char *argv[])
{
   int fd = open("myfifo", O_WRONLY);
   if(fd == -1)
   {
      perror("open error");
      exit(1);
   }

   struct event_base *base = NULL;
   base = event_base_new();
   //if(event_base_priority_init(base, 3) == -1)
   //{
   //   perror("set base error");
   //   exit(1);
   //}
 
   struct event *ev0 = NULL;
   ev0 = event_new(base, fd, EV_WRITE, write_cb0, NULL);
   //event_priority_set(ev0, 0);
   event_add(ev0, NULL);
   
   struct event *ev2 = NULL;
   ev2 = event_new(base, fd, EV_WRITE, write_cb2, NULL);
   //event_priority_set(ev2, 2);
   event_add(ev2, NULL);

   struct event *ev1 = NULL;
   ev1 = event_new(base, fd, EV_WRITE, write_cb1, NULL);
   //event_priority_set(ev1, 1);
   event_add(ev1, NULL);
   
   event_base_dispatch(base);
   event_free(ev1);
   event_free(ev2);
   event_free(ev0);
   event_base_free(base);
   return 0;
}
