/*
 * Copyright (c) 2021 <lorenzo.romio@live.it>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
*/

#pragma once

#include "../utils/common.h"
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
    uid_t             uid;
    char              name[MAX_ROOMNAME_LENGTH];
    struct user_s *   owner;
    struct user_list *userList;
    char              log_filename[MAX_ROOMNAME_LENGTH + 10];
    struct room_s *   next;
    struct room_s *   prev;
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
    char *             username;
    char *             color;
    int                sock_fd;
    struct sockaddr_in addr;
    struct room_s *    room;
    struct user_s *    next;
    struct user_s *    prev;
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
