//read_fifo.c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<event2/event.h>


//read callback function
void read_callback(evutil_socket_t fd, short what, void* arg)
{
   //read the fifo pipe
   char buf[1024] = {0};
   int len = read(fd, buf, sizeof(buf));
   printf("data len = %d, buf = %s\n", len, buf);
   printf("read event: %s\n", what & EV_READ?"Y":"N");
}


int main(int argc, const char* argv[])
{
   //construct and open a fifo pipe
   unlink("myfifo");
   mkfifo("myfifo", 0664);

   int fd = open("myfifo", O_RDONLY|O_NONBLOCK);
   if(fd == -1)
   {
      perror("open error");
      exit(1);
   }

   //read from pipr
   struct event_base* base = NULL;
   base = event_base_new();

   //construct a event
   struct event* ev = NULL;
   ev = event_new(base, fd, EV_READ, read_callback, NULL);

   //add event
   event_add(ev, NULL);

   //event loop
   event_base_dispatch(base);

   //free resource
   event_free(ev);
   event_base_free(base);
   close(fd);
   return 0;
}
