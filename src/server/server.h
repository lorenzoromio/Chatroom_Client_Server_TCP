#pragma once

#include "chat.h"

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 5000
#define ADMIN_PASSWORD "unimi"

typedef struct serverConfig
{
    char *host;
    int   port;
} serverConfig;

void sendBroadcastMessage(char *msg, user_t *user);
void sendMessage(char *msg, user_t *user);
