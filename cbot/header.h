#include "cJSON.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include <curl/curl.h>

int xsend(CURL *curl, const char *send_payload);
int handler(CURL *curl, char * data);
int presend(CURL *curl);

extern char * bot_nick;