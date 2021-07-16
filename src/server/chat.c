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

#include "chat.h"
#include "../log/logger.h"
#include "../utils/common.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <pthread.h>

// ROOM FUNCTIONS

// Inizializza la lista globale delle stanza aggiungendo la stanza generale
room_list *initializeRoomList(room_list *roomList)
{
    roomList = (room_list *)malloc(sizeof(room_list));
    if (roomList == NULL)
    {
        logError("Room list allocation");
        perror("");
        exit(EXIT_FAILURE);
    }

    roomList->roomCount = 0;
    roomList->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    roomList->head  = NULL;
    roomList->tail  = NULL;
    struct sockaddr_in admin_addr;
    admin_addr.sin_family = AF_INET;
    mkdir("chatLog", 0755);
    user_t *admin         = createUser(admin_addr, -1); //crea utente Admin con uid -1
    strcpy(admin->username, "Admin");
    addRoom(roomList, createRoom("General", admin)); //crea la stanza di default, non può essere eliminata
    removeUserFromRoom(admin);
    return roomList;
}

// Crea una stanza e ci aggiunge il suo proprietario
room_t *createRoom(char *roomName, user_t *owner)
{
    static volatile uid_t room_uid = 0;

    room_t *newRoom = (room_t *)malloc(sizeof(room_t));
    if (newRoom == NULL)
    {
        logError("Room allocation");
        perror("");
        exit(EXIT_FAILURE);
    }

    newRoom->uid   = room_uid++;
    newRoom->owner = owner;
    strcpy(newRoom->name, roomName);
    logError("");
    snprintf(newRoom->log_filename,sizeof(newRoom->log_filename),"log/%s.log",roomName);
    printf("%s\n",newRoom->log_filename);
    newRoom->userList = initializeUserList(newRoom->userList);
    newRoom->next     = NULL;
    newRoom->prev     = NULL;
    addUserToRoom(newRoom, owner);
    
    logInfo("New room created: %s", newRoom->name);
    FILE *log_fp = fopen(newRoom->log_filename, "a+");
    fclose(log_fp);
    return newRoom;
}

// Aggiunge una stanza alla lista
void addRoom(room_list *roomList, room_t *newRoom)
{
    pthread_mutex_lock(&roomList->mutex);

    newRoom->next = NULL;

    if (roomList->head == NULL)
    {
        newRoom->prev  = NULL;
        roomList->head = newRoom;
        roomList->tail = newRoom;
    }
    else
    {
        newRoom->prev        = roomList->tail;
        roomList->tail->next = newRoom;
        roomList->tail       = newRoom;
    }

    roomList->roomCount++;

    logInfo("ADD %s TO LIST", roomList->tail->name);
    pthread_mutex_unlock(&roomList->mutex);
}

// Rimuove una stanza dalla lista
void removeRoomFromList(room_list *roomList, room_t *room)
{
    // logInfo("START removeRoomFromList");
    pthread_mutex_lock(&roomList->mutex);

    if (room->uid == roomList->head->uid)
    {
        return;
    }

    logWarn("\tDeleting Room %d - %s", room->uid, room->name);

    // room is head
    if (room->prev == NULL)
    {
        // room is the only one
        if (room->next == NULL)
        {
            roomList->head = NULL;
            roomList->tail = NULL;
        }
        // room is not alone
        else
        {
            roomList->head   = room->next;
            room->next->prev = NULL;
        }
    }
    // room is tail and not alone
    else if (room->next == NULL)
    {
        roomList->tail   = room->prev;
        room->prev->next = NULL;
    }
    // room is in the middle and not alone
    else
    {
        room->prev->next = room->next;
        room->next->prev = room->prev;
    }

    pthread_mutex_unlock(&roomList->mutex);

    //Sposta tutti gli user della stanza eliminata nella stanza di default
    while (room->userList->head)
    {
        changeUserRoom(room->userList->head, roomList->head);
    }
    logWarn("DELETE ROOM [%s]", room->name);
    remove(room->log_filename);
    printf("file deleted -> %s\n", room->log_filename);
    free(room->userList);
    room->userList = NULL;
    free(room);
    room = NULL;
    roomList->roomCount--;
}

// Restituisce una stanza partendo da un uid
room_t *selectRoom(room_list *roomList, int uid)
{
    room_t *room;
    for (room = roomList->head; room; room = room->next)
    {
        if (room->uid == uid)
        {
            printf("Found [%d - %s]\n", room->uid, room->name);
            return room;
        }
    }
    logWarn("Room with uid = %d not found\n", uid);
    return NULL;
}

// Stampa la lista delle stanze con i suoi utenti
void printRoomList(room_list *roomList)
{
    // logInfo("START PRINT ROOM LIST");
    logInfo("Print room list (%d room) - (%d users)", roomList->roomCount, getTotalUserCount(roomList));

    pthread_mutex_lock(&roomList->mutex);
    room_t *room;
    for (room = roomList->head; room; room = room->next)
    {
        printf(CYAN"[%d - %s]"RESET" (%d user online) own by (%d - %s)\n",
               room->uid, room->name, room->userList->count, room->owner->uid, room->owner->username);
        printUserList(room->userList);
    }
    printf("\n");

    pthread_mutex_unlock(&roomList->mutex);
    //logInfo("END PRINT ROOM LIST");
}

// USER FUNCTIONS

// Inizializza la lista degli utenti di una stanza
user_list *initializeUserList(user_list *userList)
{
    userList = (user_list *)malloc(sizeof(user_list));
    if (userList == NULL)
    {
        logError("Room list allocation");
        perror("");
        exit(EXIT_FAILURE);
    }

    userList->count = 0;
    userList->head  = NULL;
    userList->tail  = NULL;
    userList->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    return userList;
}

// Crea un utente assegnandogli indirizzo e socket descriptor
user_t *createUser(struct sockaddr_in addr, int sock_fd)
{
    static uid_t       user_uid = 0;
    user_t *           newUser  = (user_t *)malloc(sizeof(user_t));
    static const char *colors[] = {CYAN, GREEN, RED, MAGENTA, YELLOW, BLUE, GRAY};
    int                idx      = rand() % (sizeof(colors) / sizeof(colors[0]));
    if (newUser == NULL)
    {
        logError("User allocation");
        perror("");
        exit(EXIT_FAILURE);
    }
    newUser->uid      = user_uid++;
    newUser->addr     = addr;
    newUser->sock_fd  = sock_fd;
    newUser->username = (char *)malloc(MAX_USERNAME_LENGTH + 1 * sizeof(char));
   // snprintf(newUser->ip_port, 21, "%s:%d", inet_ntoa(addr.sin_addr), addr.sin_port);
    newUser->color = (char *)malloc(10 * sizeof(char));
    strcpy(newUser->color, colors[idx]);

    return newUser;
}

// Aggiunge un utente ad una stanza
void addUserToRoom(room_t *room, user_t *newUser)
{
    user_list *userList = room->userList;
    pthread_mutex_lock(&room->userList->mutex);
    newUser->room = room;
    newUser->next = NULL;

    //empty list
    if (userList->head == NULL)
    {
        newUser->prev  = NULL;
        userList->head = newUser;
        userList->tail = newUser;
    }
    else
    {
        newUser->prev        = userList->tail;
        userList->tail->next = newUser;
        userList->tail       = newUser;
    }
    logInfo(GREEN"Insert User (%d - %s) to room [%d - %s]"RESET, newUser->uid, newUser->username, room->uid, room->name);
    room->userList->count++;

    pthread_mutex_unlock(&room->userList->mutex);

    char buffer[BUFFER_SIZE] = {'\0'};
    snprintf(buffer, BUFFER_SIZE, GREEN "++ You have been added to %s ++\n" RESET, newUser->room->name);
    sendMessage(buffer, newUser);
    bzero(buffer, sizeof(buffer));

    snprintf(buffer,BUFFER_SIZE, GREEN "++ %s has joined %s ++\n" RESET, newUser->username, newUser->room->name);
   // printf("%s", buffer);
    sendBroadcastMessage(buffer, newUser);
}

// Elimina un utente dalla lista di utenti presenti nella stanza
void removeUserFromRoom(user_t *user)
{
    if (user->room)
    {
        user_list *userList = user->room->userList;
        pthread_mutex_lock(&userList->mutex);

        // user is head
        if (user->prev == NULL)
        {
            // user is the only one
            if (user->next == NULL)
            {
                userList->head = NULL;
                userList->tail = NULL;
            }
            // user is not alone
            else
            {
                userList->head   = user->next;
                user->next->prev = NULL;
            }
        }
        // user is tail and not alone
        else if (user->next == NULL)
        {
            userList->tail   = user->prev;
            user->prev->next = NULL;
        }
        // user is in the middle and not alone
        else
        {
            user->prev->next = user->next;
            user->next->prev = user->prev;
        }

        userList->count--;

        pthread_mutex_unlock(&userList->mutex);

        logInfo(RED"Remove User (%d - %s) from room [%d - %s]"RESET, user->uid, user->username, user->room->uid, user->room->name);
        char buffer[BUFFER_SIZE];
        sendMessage(CLEAR_TERMINAL, user);
       
        snprintf(buffer, BUFFER_SIZE, RED "-- You have been removed from %s --\n" RESET, user->room->name);
        sendMessage(buffer, user);
       logDebug("ok");
        bzero(buffer, sizeof(buffer));
        snprintf(buffer,BUFFER_SIZE, RED "-- %s has left %s --\n" RESET, user->username, user->room->name);
        //printf("%s", buffer);
        sendBroadcastMessage(buffer, user);
       
        bzero(buffer, sizeof(buffer));

        user->room = NULL;

    }
}

// Sposta un utente in un altra stanza
// Ritorna FALSE se l'utente è già in quella stanza, TRUE altrimenti
int changeUserRoom(user_t *user, room_t *newRoom)
{
    if (user->room == newRoom)
    {
        return FALSE;
    }

    removeUserFromRoom(user);
    addUserToRoom(newRoom, user);
    return TRUE;
}

// Logout dell'utente, passaggio di proprietà delle sue stanza, chiusura della socket
void userLogOut(room_list *roomList, user_t *userLogOut)
{
    logInfo("Logout User (%d - %s)", userLogOut->uid, userLogOut->username);
    removeUserFromRoom(userLogOut);
    pthread_mutex_lock(&roomList->mutex);
    room_t *room;
    for (room = roomList->head; room; room = room->next)
    {
        if (room->owner == userLogOut)
        {
            logWarn("%s own %s", userLogOut->username, room->name);
            if (userLogOut->next)
            {
                room->owner = userLogOut->next;
                sendMessage(GREEN "** Now you are the owner of this room **\n" RESET, room->owner);
            }
            else
            {
                room->owner = roomList->head->owner;
                // pthread_mutex_unlock(&roomList->mutex);
                //removeRoomFromList(roomList, room);
                //pthread_mutex_lock(&roomList->mutex);
            }
        }
    }

    pthread_mutex_unlock(&roomList->mutex);
    close(userLogOut->sock_fd);
    free(userLogOut->username);
    free(userLogOut->color);
    free(userLogOut);
    //printRoomList(roomList);
}

// Stampa la lista degli utenti
void printUserList(user_list *userList)
{
    user_t *user;
    pthread_mutex_lock(&userList->mutex);
    for (user = userList->head; user; user = user->next)
    {
        printf("    %d - %s\n", user->uid, user->username);
    }

    pthread_mutex_unlock(&userList->mutex);
}

//Ritorna il numero di utenti connessi al server
unsigned int getTotalUserCount(room_list *roomList)
{
    unsigned int usersCount = 0;
    room_t *     room;
    pthread_mutex_lock(&roomList->mutex);
    for (room = roomList->head; room; room = room->next)
    {
        usersCount += room->userList->count;
    }
    roomList->usersCount = usersCount;
    pthread_mutex_unlock(&roomList->mutex);
    return usersCount;
}