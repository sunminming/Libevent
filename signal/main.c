#include <unistd.h>
#include <signal.h>
#include <stdio.h>

void fun_sigint(int sig)
{
   printf("This is in fun_sigint\n");
}

int main(int argc, char **argv)
{
   struct sigaction act, oact;
   
   sigset_t newsigset, oldsigset, pendsigset;
   sigemptyset(&newsigset);
   sigemptyset(&oldsigset);
   sigemptyset(&pendsigset);

   sigpending(&pendsigset);
   if(sigismember(&pendsigset, SIGINT)) printf("SUGINT is pending\n");

   sigaddset(&newsigset, SIGINT);
   sigprocmask(SIG_BLOCK, &newsigset, &oldsigset);

   act.sa_handler = fun_sigint;
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   sigaction(SIGINT, &act, &oact);
   sleep(5);

   sigpending(&pendsigset);
   if(sigismember(&pendsigset, SIGINT)) printf("SUGINT is pending\n");
}