#pragma once

#include <arpa/inet.h>
#include <pthread.h>

#define MAX_ROOM_COUNT   10
#define MAX_CLIENT_COUNT 1000
#define ROOM_SIZE        250


typedef struct room_list
{
    volatile unsigned int roomCount;
    volatile unsigned int usersCount;
    struct room_s *       head;
    struct room_s *       tail;
    pthread_mutex_t       mutex;

} room_list;

// Struttura della stanza
typedef struct room_s
{
    uid_t              uid;
    char               name[25];
    struct user_s *    owner;
    struct user_list * userList;
    char               log_filename[30];
    struct room_s *    next;
    struct room_s *    prev;
} room_t;

// Struttura dell'utente
typedef struct user_list
{
    volatile unsigned int count;
    struct user_s *       head;
    struct user_s *       tail;
    pthread_mutex_t       mutex;

} user_list;

typedef struct user_s
{
    uid_t              uid;
    char               *username;
    char               *color;
    int                sock_fd;
    struct sockaddr_in addr;
    struct room_s *    room;
    struct user_s *    next;
    struct user_s *    prev;
    //char               ip_port[21];

} user_t;

//ROOM FUNCTIONS

room_list *initializeRoomList(room_list *roomList);

room_t *createRoom(char *roomName, user_t *owner);

void addRoom(room_list *roomList, room_t *newRoom);

void removeRoomFromList(room_list *roomList, room_t *roomToRemove);

room_t *selectRoom(room_list *roomList, int uid);

void printRoomList(room_list *roomList);

//USER FUNCTIONS

user_list *initializeUserList(user_list *userList);

user_t *createUser(struct sockaddr_in addr, int sock_fd);

void addUserToRoom(room_t *room, user_t *newUser);

void removeUserFromRoom(user_t *user);

int changeUserRoom(user_t *user, room_t *newRoom);

void userLogOut(room_list *roomList, user_t *userLogOut);

void printUserList(user_list *userList);

unsigned int getTotalUserCount(room_list *roomList);
