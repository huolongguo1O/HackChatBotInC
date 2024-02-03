#include "header.h"

char * help_data ="Help for cbot:\n"
"ping: if 'ping' in your message, cbot will reply 'pong';\n"
"help: show the help menu;\n"
"afk: mark as afk;\n";
char * bot_nick = "cbot";

struct afk_list{
    char * nick;
    char * reason;
    struct afk_list * next;
};


struct afk_list * afk_start;
struct afk_list * afk_end;

int presend(   //You can send something after connection
    CURL * curl,
)
{
    xsend(curl, "{\"cmd\":\"join\",\"channel\":\"lounge\",\"nick\":\"cbot\"}"); //see handler.c:6
    xsend(curl, "{\"cmd\":\"chat\",\"text\":\"I'm using c\"}");
    return 0;
}
int handler(    //ws data handler (will be called from ws-level)
    CURL * curl, //curl handle.
    char * data // data recved from server (JSON encoded).
)
{
    cJSON * json= cJSON_Parse(data);
    if(json == 0){
        return 0;
    }
    cJSON * _cmd = cJSON_GetObjectItem(json, "cmd");
    if(_cmd == 0){
        return 0;
    }
    char * cmd = _cmd -> valuestring;
    if (strcmp(cmd, "chat") == 0){
        cJSON * _nick = cJSON_GetObjectItem(json, "nick");
        if(strlen(_nick) > 24) return 0; // prevent more than 24-byte nick when fuzzing
        if(strcmp(_nick -> valuestring, bot_nick)==0){
            return 0;
        }
        cJSON * _text = cJSON_GetObjectItem(json, "text");
        if(strstr(_text -> valuestring, "ping") == _text -> valuestring){
            xsend(curl, "{\"cmd\":\"chat\",\"text\":\"pong\"}");
        }
        if(strstr(_text -> valuestring, "help") == _text -> valuestring){
            cJSON *buf = cJSON_CreateObject();
            cJSON_AddStringToObject(buf,"cmd","chat");
            cJSON_AddStringToObject(buf,"text",help_data);
            printf("%s\n",cJSON_Print(buf));
            xsend(curl, cJSON_Print(buf));
        }
        if(strstr(_text -> valuestring, bot_nick) == _text -> valuestring + 1){
            cJSON *buf = cJSON_CreateObject();
            char _buf[256] = "@";
            strcat(_buf, _nick -> valuestring);
            strcat(_buf, " ");
            strcat(_buf, "why do u @me");
            cJSON_AddStringToObject(buf,"cmd","chat");
            cJSON_AddStringToObject(buf,"text",_buf);
            printf("%s\n",cJSON_Print(buf));
            xsend(curl, cJSON_Print(buf));
        }
        if(strstr(_text -> valuestring, "afk") == _text -> valuestring){
            struct afk_list * _afk = (struct afk_list *)malloc(sizeof(struct afk_list));
            _afk -> nick = _nick -> valuestring;
            if(afk_end)
                afk_end -> next = _afk;
            afk_end = _afk;
            _afk -> next = NULL;
            if(!afk_start){
                afk_start = _afk;
            }
            cJSON *buf = cJSON_CreateObject();
            cJSON_AddStringToObject(buf,"cmd","chat");
            cJSON_AddStringToObject(buf,"text","You are marked as afk.");
            xsend(curl, cJSON_Print(buf));
            return 0;
        }
        struct afk_list * afk_temp = afk_start;
        while(afk_temp){
            if(strstr(_text -> valuestring, afk_temp -> nick)){
                cJSON *buf = cJSON_CreateObject();
                char _buf[256] = "@";
                sprintf(_buf,"@%s @%s is now afk.", _nick -> valuestring, afk_temp-> nick);
                cJSON_AddStringToObject(buf,"cmd","chat");
                cJSON_AddStringToObject(buf,"text",_buf);
                printf("%s\n",cJSON_Print(buf));
                xsend(curl, cJSON_Print(buf));
                break;
            }
        }
        

    }
    struct afk_list * _afk = afk_start;
    cJSON * _nick = cJSON_GetObjectItem(json, "nick");
    if(_nick == 0 || !strcmp(_cmd , "onlineRemove")){
        goto NO_AFK_CHECK;
    }
    char * nick = _nick -> valuestring;
    struct afk_list * _last_afk = 0;
    while(_afk){
        if(strcmp(nick, _afk -> nick) == 0){
            cJSON *buf = cJSON_CreateObject();
            cJSON_AddStringToObject(buf,"cmd","chat");
            cJSON_AddStringToObject(buf,"text","Welcome back.");
            xsend(curl, cJSON_Print(buf));
            if(_last_afk){
                _last_afk -> next = _afk -> next;
                free(_afk);
            }else{ // if the first
                afk_start = 0;
                afk_end = 0;
                free(_afk);
            }
            
            break;
        }
        _last_afk = _afk;
        _afk = _afk -> next;
    }
    
NO_AFK_CHECK:
    cJSON_Delete(json);
    // cJSON_free?
    return 0;
}