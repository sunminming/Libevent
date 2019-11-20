//write_fifo.c
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<event2/event.h>

//callback function
void write_callback(evutil_socket_t fd, short what, void *arg)
{
   //write into pipe
   char buf[1024] = {0};
   static int num = 666;
   sprintf(buf, "hello, world == %d\n", num);
   //gets(buf);
   write(fd, buf, strlen(buf)+1);
}

int main(int argc, const char* argv[])
{
   //open the fifo file
   int fd = open("myfifo", O_WRONLY|O_NONBLOCK);
   if(fd == -1)
   {
      perror("open error");
      exit(1);
   }

   //construct a event_base
   struct event_base* base = NULL;
   base = event_base_new();

   //construct a event
   struct event* ev = NULL;
   ev = event_new(base, fd, EV_WRITE, write_callback, NULL);

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
