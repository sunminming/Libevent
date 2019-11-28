#include <event2/event.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, const char *argv[])
{
   int fd = open("myfifo", O_WRONLY|O_NONBLOCK);
   if(fd == -1)
   {
      perroe("open error");
      exit(1);
   }

   struct event_base *base = NULL;
   base = event_base_new();
   if(event_base_priority_init(base, 3) == -1)
   {
      perror("set base error");
      exit(1);
   }
 
   struct event *ev1 = NULL;
   ev1 = event_new(base, fd, EV_WRITE, write_cb1, NULL);
   event_priority_set(ev1, 1);
   event_add(ev1, NULL);
   struct event *ev3 = NULL;
   ev3 = event_new(base, fd, EV_WRITE, write_cb3, NULL);
   event_priority_set(ev3, 3);
   event_add(ev3, NULL);
   struct event *ev2 = NULL;
   ev2 = event_new(base, fd, EV_WRITE, write_cb2, NULL);
   event_priority_set(ev2, 2);
   event_add(ev2, NULL);
   event_base_dispatch(base);
   event_free(ev1);
   event_free(ev2);
   event_free(ev3);
   event_base_free(base);
   return 0;
}
