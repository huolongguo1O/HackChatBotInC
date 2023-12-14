
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "header.h"
#include <pthread.h>
extern char http_proxy[16384][32];
extern char socks5_proxy[16384][32];
extern char socks4_proxy[16384][32];
char ** get_proxy_http(int * total);
char ** get_proxy_socks5(int * total);
char ** get_proxy_socks4(int * total);
char * get_join_command(){
    char * nick = (char *) malloc(512);
    int nickraw = rand();
    int nickraw2 = rand();
    sprintf(nick, "{\"cmd\":\"join\",\"channel\":\"openmine\",\"nick\":\"%d%d\",\"password\":\"%d%d\"}", nickraw, nickraw2, nickraw, nickraw2);
    return nick;
}
static int ping(CURL *curl, const char *send_payload)
{
  size_t sent;
  CURLcode result =
    curl_ws_send(curl, send_payload, strlen(send_payload), &sent, 0,
                 CURLWS_PING);
  return (int)result;
}

int xsend(CURL *curl, const char *send_payload)
{
  size_t sent;
  CURLcode result =
    curl_ws_send(curl, send_payload, strlen(send_payload), &sent, 0,
                 CURLWS_TEXT);
  return (int)result;
}

static int recv_pong(CURL *curl, const char *expected_payload)
{
  size_t rlen;
  const struct curl_ws_frame *meta;
  char buffer[256];
  CURLcode result = curl_ws_recv(curl, buffer, sizeof(buffer), &rlen, &meta);
  if(!result) {
    if(meta->flags & CURLWS_PONG) {
      int same = 0;
      fprintf(stderr, "ws: got PONG back\n");
      if(rlen == strlen(expected_payload)) {
        if(!memcmp(expected_payload, buffer, rlen)) {
          fprintf(stderr, "ws: got the same payload back\n");
          same = 1;
        }
      }
      if(!same)
        fprintf(stderr, "ws: did NOT get the same payload back\n");
    }
    else {
      fprintf(stderr, "recv_pong: got %u bytes rflags %x\n", (int)rlen,
              meta->flags);
    }
  }
  fprintf(stderr, "ws: curl_ws_recv returned %u, received %u\n",
          (unsigned int)result, (unsigned int)rlen);
  return (int)result;
}

static int recv_any(CURL *curl)
{
  size_t rlen;
  const struct curl_ws_frame *meta;
  char *buffer = malloc(sizeof(char)*65536*1024);
  if(buffer == NULL){
    return -1;
  }
  CURLcode result = curl_ws_recv(curl, buffer, 65536*1024, &rlen, &meta);
  //printf("DEBUG: result = %d, %s\n",result,curl_easy_strerror(result));
  if(result && result != CURLE_AGAIN)
    return result;
  while(result == CURLE_AGAIN){
    //sleep(1);
    result = curl_ws_recv(curl, buffer, 65536*1024, &rlen, &meta);
    //return 0;
  }
  //printf("%s\n",buffer);
  if(buffer[8]=='w'){
    printf("Got rl!!");
    sleep(60);
    }
  //handler(curl, buffer);
  free(buffer);
  if(result)
    return result;

  return 0;
}

/* close the connection */
static void websocket_close(CURL *curl)
{
  size_t sent;
  (void)curl_ws_send(curl, "", 0, &sent, 0, CURLWS_CLOSE);
}

static void websocket(CURL *curl,char *proxy)
{
  int i = 0;
  char * buf;
  buf = get_join_command();
  xsend(curl, buf); //see handler.c:6
  //printf("%s------%s\n",buf,proxy);
  //xsend(curl, "{\"cmd\":\"chat\",\"text\":\"I'm using c\"}");
  recv_any(curl);
  free(buf);
  websocket_close(curl);
}

int main_t(char * proxy)
{
  //float failure_rate = 0.0;
  float failure = 0.0;
  float total = 0.0;
    while(1){
      total++;
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "wss://hack.chat/chat-ws");
    // set proxy
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 45);
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L); /* websocket style */

    /* Perform the request, res will get the return code */
    //while(1){
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK){
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
      sleep(60*(failure/total));
      failure++;
      if((failure/total > 0.8 && total > 10)/*||
      (failure==total && total > 5)*/){
        printf("failure rate: %f-%s\n", failure/total, proxy);
        //exit(1);
        return res;

      }
    }

    else {
      /* connected and ready */
      websocket(curl, proxy);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    sleep(25);
    }
  }
  return 0;
}

int main(){
    //char ** proxies;
    int ptot = 0;
    /*proxies = */get_proxy_http(&ptot);
    srand(time(NULL));
    for(int i=0; i<ptot; i++){
        // run main_t in a new thread
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)main_t, http_proxy[i]);
        //pthread_detach(thread);

    }
    ptot=0;
    /*proxies = */get_proxy_socks5(&ptot);
    //srand(time(NULL));
    for(int i=0; i<ptot; i++){
        // run main_t in a new thread
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)main_t, socks5_proxy[i]);
        //pthread_detach(thread);

    }
    ptot=0;
    /*proxies = */get_proxy_socks4(&ptot);
    //srand(time(NULL));
    for(int i=0; i<ptot; i++){
        // run main_t in a new thread
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)main_t, socks4_proxy[i]);
        //pthread_detach(thread);

    }
    while(1)
      sleep(3000);
}
