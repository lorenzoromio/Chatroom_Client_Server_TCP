#pragma once

#define _STR(x) #x
#define STR(x)  _STR(x)

#define CLEAR_TERMINAL      "\033[1;1H\033[2J"
#define CLEAR_LINE          "\33[2K"
#define CURSOR_UP           "\033[A"
#define MIN_USERNAME_LENGTH 3
#define MAX_USERNAME_LENGTH 25
#define MIN_ROOMNAME_LENGTH 3
#define MAX_ROOMNAME_LENGTH 25
#define BUFFER_SIZE         2048
#define MAX_MESSAGE_LENGTH  2000
#define TRUE                1
#define FALSE               0