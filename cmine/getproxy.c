#include <stdio.h>
#include <string.h>
char http_proxy[16384][32]={0};
char socks5_proxy[16384][32]={0};
char socks4_proxy[16384][32]={0};
void get_proxy_http(int * total)
{
    FILE * fp;
    fp = fopen("http.txt","r");
    if(fp == NULL)
    {
        printf("http.txt not found\n");
        //return NULL;
    }
    for(int i = 0;i < 16384;i++){
        if(fscanf(fp,"%s",http_proxy[i]) == EOF){
            *total = i+1;
            break;
        }
    }
    // return http_proxy;
    //return 0;
}

void get_proxy_socks5(int * total)
{
    FILE * fp;
    fp = fopen("socks5.txt","r");
    if(fp == NULL)
    {
        printf("socks5.txt not found\n");
        //return NULL;
    }
    for(int i = 0;i < 16384;i++){
        strcpy(socks5_proxy[i],"socks5://");
        if(fscanf(fp,"%s",socks5_proxy[i]+9) == EOF){
            *total = i+1;
            break;
        }
    }
    // return socks5_proxy;
    //return 0;
}

void get_proxy_socks4(int * total)
{
    FILE * fp;
    fp = fopen("socks4.txt","r");
    if(fp == NULL)
    {
        printf("socks4.txt not found\n");
        //return NULL;
    }
    for(int i = 0;i < 16384;i++){
        strcpy(socks4_proxy[i],"socks4://");
        if(fscanf(fp,"%s",socks4_proxy[i]+9) == EOF){
            *total = i+1;
            break;
        }
    }
    //return socks4_proxy;
    //return 0;
}