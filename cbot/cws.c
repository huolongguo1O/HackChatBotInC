// This is websocket driver copied from curl.se
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "header.h"

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
    result = curl_ws_recv(curl, buffer, 65536*1024, &rlen, &meta);
  }
  printf("%s\n",buffer);
  handler(curl, buffer);
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

static void websocket(CURL *curl)
{
  int i = 0;
  // joining cmd
  // considering move to handler.c
  // xsend(curl, "{\"cmd\":\"join\",\"channel\":\"lounge\",\"nick\":\"cbot\"}"); //see handler.c:6
  // xsend(curl, "{\"cmd\":\"chat\",\"text\":\"I'm using c\"}");
  presend(curl);
  //sleep(3);
  while(1) {
    int ret = recv_any(curl);
    if(ret){
      printf("Error! Abort!");
      abort();
      return;
    }
  }
  do {
    recv_any(curl);
    if(ping(curl, "foobar"))
      return;
    if(recv_pong(curl, "foobar")) {
      return;
    }
    sleep(2);
  } while(i++ < 10);
  websocket_close(curl);
}

int main(void)
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "wss://hack.chat/chat-ws"); // websocket address

    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L); /* websocket style */

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    else {
      /* connected and ready */
      websocket(curl);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}
