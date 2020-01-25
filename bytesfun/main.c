#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
   printf("This program is used to show the role of several byte order conversion functions\n");
   printf("--------------------------------------------------------------------------------\n");
   uint16_t host_s;
   host_s = atoi(argv[1]);
   
   printf("Function htons and ntohs:\n");
   uint16_t network_s = htons(host_s);
   printf("\t%hu after fun htons: %hu\n", host_s, network_s);

   host_s = ntohs(network_s);
   printf("\t%hu after fun ntohs: %hu\n", network_s, host_s);

   printf("Function inet_aton and inet_ntoa:\n");
   struct sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));
   char *host_c = argv[2];
   if(inet_aton(host_c, &(addr.sin_addr)));
   {
      printf("\t%s after fun inet_aton: %u\n", host_c, addr.sin_addr.s_addr);
   }

   printf("\t%u after fun inet_ntoa: %s\n", addr.sin_addr.s_addr, 
		   inet_ntoa(addr.sin_addr)); 
   
   memset(&addr, 0, sizeof(addr));
   printf("Function inet_pton and inet_ntop:\n");
   if((inet_pton(AF_INET, host_c, &(addr.sin_addr.s_addr))) == 1)
   {
      printf("\t%s after fun inet_pton: %u\n", host_c, addr.sin_addr.s_addr);
   }

   printf("\t%u after fun inet_ntop: %s\n", addr.sin_addr.s_addr, 
		   inet_ntop(AF_INET, &(addr.sin_addr.s_addr), host_c, INET_ADDRSTRLEN));
   
   return 0;
}
