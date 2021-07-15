#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define CONNECTION_HOST "127.0.0.1"
#define CONNECTION_PORT 5000

int resolveHostname(char *hostname, char *ip);

typedef struct 
{
    char *host;
    int port;
} clientConfig;



