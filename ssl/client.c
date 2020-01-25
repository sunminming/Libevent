#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include "basicev.h"
 
static void read_cb(struct bufferevent *bev, void *arg)
{
   char buf[1024] = {0};
   bufferevent_read(bev, buf, 1024);
   printf("%s\n", buf);
}

int main(int argc, char **argv)
{
   int sockfd, len;
   struct sockaddr_in dest;
   SSL_CTX *ctx;
   SSL *ssl;
 
   /* SSL 库初始化，参看 ssl-server.c 代码 */
   SSL_library_init();
   OpenSSL_add_all_algorithms();
   ERR_load_crypto_strings();
   SSL_load_error_strings();
   ctx = SSL_CTX_new(SSLv23_client_method());
   if (ctx == NULL) {
      ERR_print_errors_fp(stdout);
      exit(1);
   }
    
   // 创建一个 socket 用于 tcp 通信 
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      printf("Socket: %s\n", strerror(errno));
      exit(1);
   }
    
   // 初始化服务器端（对方）的地址和端口信息 
   memset(&dest, 0 ,sizeof(dest));
   dest.sin_family = AF_INET;
   dest.sin_port = htons(9999);
   if (inet_aton("127.0.0.1", &dest.sin_addr) == 0) {
      exit(errno);
   }
    
   //init event2
   struct event_base *base = NULL;
   struct bufferevent *sslbev = NULL;

   base = event_base_new();
   if(base == NULL)
   {
      printf("%s\n", strerror(errno));
      exit(1);
   }
   // 连接服务器 
    
   if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
      printf("Connect: %s\n ", strerror(errno));
      exit(errno);
   }
   
   /* 基于 ctx 产生一个新的 SSL */
   ssl = SSL_new(ctx);
   //sslbev = bufferevent_openssl_socket_new(base, sockfd, ssl,
   //		    BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
   
   struct bufferevent *bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);
   sslbev = bufferevent_openssl_filter_new(base, bev, ssl, 
		   BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
   // 接收对方发过来的消息，最多接收 MAXBUF 个字节
   bufferevent_setcb(sslbev, read_cb, NULL, NULL, NULL);
   bufferevent_enable(sslbev, EV_READ|EV_WRITE);
   add_signal(base);
   add_stdin(base, sslbev);
   event_base_dispatch(base);

   bufferevent_free(sslbev);
   event_base_free(base);  
   SSL_CTX_free(ctx);
   return 0;
}
