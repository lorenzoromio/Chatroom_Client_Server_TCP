#include "server.h"
#include "../log/logger.h"
#include "../utils/common.h"
#include "../utils/utils.h"
#include "chat.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <pthread.h>
#include <signal.h>
#include <syscall.h>
#include <time.h>

void *handleClient(void *arg);
void  signalHandler(int sig);
void *handleServerCommand();

void sendMessage(char *msg, user_t *user);
void sendBroadcastMessage(char *msg, user_t *user);
void keepLog(char *msg, user_t *user);
void restoreLog(user_t *user);

void help(user_t *user);
void helpListRooms(user_t *user);
void helpListUsers(room_t *room, user_t *user);
void helpNewRoom(user_t *user);
int  cleanEmptyRooms(room_list *roomList);
void helpChangeRoom(user_t *user);
void helpDeleteRoom(room_list *roomList, room_t *room, user_t *user);
int  isAdminLogged(room_list *roomList);

static room_list *roomList = NULL;
// Utenti collegati in attesa di inserire username per accedere ad una stanza
static volatile int waitingForConnection = 0;

int main(int argc, char **argv)
{
    int                server_sd = 0, client_sd = 0;
    serverConfig       config;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int                option;
    int                status;
    pthread_t          threadId;

    //    increaseLogLevel();
    //    increaseLogLevel();

    // Gestione dei segnali attraverso una funzione definita
    signal(SIGINT, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGQUIT, signalHandler);

    // default
    config.host = SERVER_HOST;
    config.port = SERVER_PORT;
    parserArgv(argc, argv, &config);

    // Assegno indirizzo locale alla socket
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(config.host);
    server_addr.sin_port        = htons(config.port);

    // Socket setting TCP
    server_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sd < 0)
    {
        logError("Socket creation failed");
        perror("");
        return EXIT_FAILURE;
    }

    /*
    option = 1;
    status = setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
    if (status < 0)
    {
        logError("Setsockopt SO_REUSEADDR failed");
        perror("");
        return EXIT_FAILURE;
    }

*/

    /*
    struct linger sl;
    sl.l_onoff  = 1; 
    sl.l_linger = 0;
    status      = setsockopt(server_sd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
    if (status < 0)
    {
        logError("Setsockopt SO_LINGER failed");
        perror("");
        return EXIT_FAILURE;
    }
*/

    // Imposto dimensione del buffer di ricezione e di invio
    option = BUFFER_SIZE;
    status = setsockopt(server_sd, SOL_SOCKET,
                        (SO_SNDBUF | SO_RCVBUF), (char *)&option, sizeof(option));

    logDebug("setsockopt status = %d", status);
    if (status < 0)
    {
        logError("Setsockopt SO_SNDBUF | SO_RCVBUF failed");
        perror("");
        return EXIT_FAILURE;
    }

    // Bind
    status = bind(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    logDebug("bind status = %d", status);
    if (status < 0)
    {
        logError("Socket binding failed");
        perror("");
        return EXIT_FAILURE;
    }

    // Listen
    status = listen(server_sd, 10);
    logDebug("listen status = %d", status);
    if (status < 0)
    {
        logError("Socket listening failed");
        perror("");
        return EXIT_FAILURE;
    }

    roomList = initializeRoomList(roomList);

    pthread_create(&threadId, NULL, &handleServerCommand, NULL);

    printf(CLEAR_TERMINAL);
    printf("=== WELCOME TO THE CHATROOM SERVER ===\n");
    printf("Listening for connection on %s:%d\n",
           config.host, config.port);
    printf("\n"
        "? -- Print this menu\n"
        "p -- Print room list\n"
        "c -- Clear screen\n"
        "q -- Quit\n");

    // TODO remove me
    // printf("Use  \"./client %s %d\" or \"telnet %s -p %d\" for connecet\n",
    //       config.host, config.port, config.host, config.port);

    while (1)
    {
        socklen_t clilen = sizeof(client_addr);
        client_sd        = accept(server_sd, (struct sockaddr *)&client_addr, &clilen);

        waitingForConnection++;

        //Controllo se posso accettare nuovi utenti nel server
        if ((getTotalUserCount(roomList) + waitingForConnection) > MAX_CLIENT_COUNT)
        {
            logWarn("Max clients reached. Rejected connection from %s:%d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
            waitingForConnection--;
            close(client_sd);
            continue;
        }

        logInfo("Connection received from %s:%d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        // Manda conferma di connessione
        send(client_sd, STR(TRUE), 2, 0);

        //Crea un utente
        user_t *user = createUser(client_addr, client_sd);

        //Lancio il thread per la gestione dell'utente
        pthread_create(&threadId, NULL, &handleClient, (void *)user);

        /* Reduce CPU usage */
        usleep(1000);
    }

    return EXIT_SUCCESS;
}

void *handleServerCommand()
{

    // questo rende stin immediato
    struct termios oldTeminal, newTeminal;
    tcgetattr(STDIN_FILENO, &oldTeminal);
    newTeminal = oldTeminal;
    newTeminal.c_lflag &= ~(ICANON);
    logDebug("Terminal set attributes %d", tcsetattr(STDIN_FILENO, TCSANOW, &newTeminal));

    while (1)
    {
        char command = getchar();
        printf("\r");
        switch (command)
        {
        case 'p':
        case 'P':
            printRoomList(roomList);
            break;
        case 'c':
        case 'C':
            printf(CLEAR_TERMINAL);
            printf("=== WELCOME TO THE CHATROOM SERVER ===\n");
            break;
        case 'Q':
        case 'q':
            signalHandler(4);
            break;
        case '?':
            printf(
                "? -- Print this menu\n"
                "p -- Print room list\n"
                "c -- Clear screen\n"
                "q -- Quit\n\n");
            break;
        case '\n':
            printf(CURSOR_UP);
            break;

        default:
            logError("Command " CYAN "'%c'" RESET " not defined" CURSOR_UP, command);
            break;
        }
    }
}

// Gestisce la comunicazione con il client in un thread separato
void *handleClient(void *arg)
{
    int  receive = 0;
    char buffer[BUFFER_SIZE];

    // Se impostata a TRUE esegue il logOut del client
    int leave_flag = FALSE;

    user_t *user = (user_t *)arg;
    sendMessage(CLEAR_TERMINAL YELLOW "=== WELCOME TO THE CHATROOM ===\n" RESET, user);
    sendMessage("Please enter your username (length: " STR(MIN_USERNAME_LENGTH) " - " STR(MAX_USERNAME_LENGTH) ")> ", user);

    // Name
    receive = recv(user->sock_fd, buffer, MAX_USERNAME_LENGTH + 1, 0);
    strcpy(buffer, stripString(buffer));
    if (receive <= 0)
    {
        perror("");
        logWarn("Client disconnected before login!");
        strcpy(user->username, "NULL");
        leave_flag = TRUE;
    }
    else
    {
        logDebug("Client tried username %s => buffer [%.*s]",
                 buffer, MAX_USERNAME_LENGTH, buffer);

        if (strlen(buffer) < MIN_USERNAME_LENGTH || strlen(buffer) > MAX_USERNAME_LENGTH)
        {
            logError("Client username invalid! Closing connection...");
            leave_flag = TRUE;
        }
        else
        {
            strcpy(user->username, stripString(buffer));
            if (strcmp(user->username, roomList->head->owner->username) == 0)
            {
                if (!isAdminLogged(roomList))
                {
                    logInfo("Admin is logged");
                    usleep(200);
                    sendMessage(MAGENTA "Insert Admin Password:\n" RESET, user);
                    logInfo("Waiting for password");
                    receive = recv(user->sock_fd, buffer, MAX_USERNAME_LENGTH + 1, 0);
                    if (receive > 0)
                    {
                        strcpy(buffer, stripString(buffer));
                        if (strcmp(buffer, ADMIN_PASSWORD) == 0)
                        {
                            user->uid = 0;
                            sendMessage(CLEAR_TERMINAL YELLOW "=== WELCOME TO THE CHATROOM ===\n" RESET, user);

                            help(user);
                            addUserToRoom(roomList->head, user);
                        }
                        else
                        {
                            logError("Wrong password");
                            leave_flag = TRUE;
                        }
                    }
                    else
                    {
                        logError("Receive no data");
                        perror("");
                        leave_flag = TRUE;
                    }
                }
                else
                {
                    usleep(200);
                    sendMessage(RED "Admin already logged.\n" RESET, user);
                    logWarn("Admin already logged");
                    leave_flag = TRUE;
                }
            }
            else
            {
                sendMessage(CLEAR_TERMINAL YELLOW "=== WELCOME TO THE CHATROOM ===\n" RESET, user);
                addUserToRoom(roomList->head, user);
                help(user);
            }
        }
    }

    waitingForConnection--;

    bzero(buffer, BUFFER_SIZE);

    while (!leave_flag)
    {
        char msg[BUFFER_SIZE + 32] = {'\0'};
        snprintf(msg, sizeof(msg), "%s > ", user->username);
        sendMessage("> ", user);
        bzero(msg, sizeof(msg));

        int receive = recv(user->sock_fd, buffer, BUFFER_SIZE, 0);

        printf(GRAY "[Thread = %li] " RESET, syscall(SYS_gettid));
        fflush(stdout);
        strcpy(buffer, stripString(buffer));
        if (receive > 0)
        {
            if (strlen(buffer) > MAX_MESSAGE_LENGTH)
            {
                buffer[MAX_MESSAGE_LENGTH + 1] = '\n';
                buffer[MAX_MESSAGE_LENGTH + 2] = '\0';
                char errorMsg[]                = RED "[ERROR] Message too long! Truncate at " STR(MAX_MESSAGE_LENGTH) " characters\n " RESET;
                sendMessage(errorMsg, user);
                printf("%s", errorMsg);
            }
            if (strlen(buffer) > 0)
            {
                if (buffer[0] == '/') // il messaggio è un comando
                {
                    if (strcmp(buffer, "/help") == 0)
                    {
                        help(user);
                    }
                    else if (strcmp(buffer, "/rooms") == 0)
                    {
                        helpListRooms(user);
                    }
                    else if (strcmp(buffer, "/newroom") == 0)
                    {
                        helpNewRoom(user);
                    }
                    else if (strcmp(buffer, "/deleteroom") == 0)
                    {
                        helpDeleteRoom(roomList, user->room, user);
                    }
                    else if (strcmp(buffer, "/changeroom") == 0)
                    {
                        helpChangeRoom(user);
                    }
                    else if (strcmp(buffer, "/restore") == 0)
                    {
                        restoreLog(user);
                    }
                    else if (strcmp(buffer, "/exit") == 0)
                    {
                        leave_flag = TRUE;
                    }
                    else
                    {
                        sendMessage(RED "Command not found:\n" RESET, user);
                        help(user);
                    }
                }
                else // Messaggio normale
                {
                    // Aggiungo data e ora di invio messaggio
                    time_t     timer;
                    char       timeStr[20];
                    struct tm *tm_info;

                    timer   = time(NULL);
                    tm_info = localtime(&timer);

                    strftime(timeStr, 26, "[%d/%m/%y %H:%M:%S]", tm_info);

                    snprintf(msg, sizeof(msg), "%s%s [%s]: " RESET "%s\n", user->color, timeStr, user->username, buffer);
                    printf("[%s] %s", user->room->name, msg);
                    sendBroadcastMessage(msg, user);
                }
                bzero(msg, sizeof(msg));
            }
        }
        else if (receive == 0)
        {
            logError("(%d - %s) CRASHED", user->uid, user->username);
            //TODO quando l'user crasha il server si chuide!! fuck!! solo sul raspberry!!
            leave_flag = TRUE;
        }
        else
        {
            logError("-1");
            perror("");
            leave_flag = TRUE;
        }

        bzero(buffer, BUFFER_SIZE);
    }

    // Logout user e elimina il thread

    userLogOut(roomList, user);
    pthread_detach(pthread_self());

    return NULL;
}

// Gestione dei segnali di interrupt
void signalHandler(int sig)
{
    room_t *   room;
    user_list *userList;
    user_t *   user;

    for (room = roomList->tail; room; room = room->prev)
    {
        userList = room->userList;
        for (user = userList->tail; user; user = user->prev)
        {
            userLogOut(roomList, user);
        }
        removeRoomFromList(roomList, room);
    }

    switch (sig)
    {
    case 20:
        logInfo("CTRL-Z DETECTED");
        break;
    case 2:
        logInfo("CTRL-C DETECTED");
        break;
    case 4:
        logInfo("QUIT");
        break;
    default:
        logInfo("SIG %d DETECTED", sig);
        break;
    }

    exit(sig);
}

// Manda un messaggio direttamente ad un utente
void sendMessage(char *msg, user_t *user)
{
    //str_trim_lf(msg, sizeof(msg));

    /*
    logError("Sending message '%s' to user %d - %s at ip %s:%d",
             msg,
             user->uid, user->username,
             inet_ntoa(user->addr.sin_addr),
             user->addr.sin_port);
*/
    if (send(user->sock_fd, msg, strlen(msg), 0) < 0)
    {

        logError("Couldn't send message '%s' to user %d - %s at ip %s:%d",
                 msg,
                 user->uid, user->username,
                 inet_ntoa(user->addr.sin_addr),
                 user->addr.sin_port);

        perror("");
    }
}

// Manda un messaggio a tutti gli utenti della stanza dell'utente eccetto il mittente
void sendBroadcastMessage(char *msg, user_t *user)
{
    str_trim_lf(msg, sizeof(msg));
    room_t *room = user->room;

    pthread_mutex_lock(&room->userList->mutex);

    user_t *recipient;

    for (recipient = room->userList->head; recipient; recipient = recipient->next)
    {
        if (user->uid != recipient->uid)
        {
            sendMessage(msg, recipient);
        }
    }
    pthread_mutex_unlock(&room->userList->mutex);
    keepLog(msg, user);
}

// Salva i messaggi nel file di log della stanza
void keepLog(char *msg, user_t *user)
{

    FILE *log_fp = fopen(user->room->log_filename, "a+"); // a+ (create + append) option will allow appending which is useful in a log file
    if (log_fp == NULL)
    {
        perror("[-] Error during logging");
    }

    fprintf(log_fp, "%s", msg);
    fclose(log_fp);
}

// Ripristina la cronologia dei messaggi della stanza in cui è l'utente
void restoreLog(user_t *user)
{
    logInfo("restore log for (%d)%s", user->uid, user->username);
    FILE *log_fd              = fopen(user->room->log_filename, "r");
    char  buffer[BUFFER_SIZE] = {'\0'};
    if (log_fd == NULL)
    {
        logError("could not open file %s", user->room->log_filename);
    }

    while (fgets(buffer, sizeof(buffer), log_fd))
    {
        sendMessage(buffer, user);
        bzero(buffer, sizeof(buffer));
    }
}

// Manda la lista dei comandi al client
void help(user_t *user)
{
    logInfo("%s requests help", user->username);
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, MAGENTA "Help Options:\n" RESET "\t/help\t\tShow help options\n"
                                          "\t/restore\tRestore chat chronology\n"
                                          "\t/rooms\t\tPrint room list\n"
                                          "\t/newroom\tCreate a room\n"
                                          "\t/deleteroom\tDelete current room (owner only)\n"
                                          "\t/changeroom\tSelect a different room\n"
                                          "\t/exit\t\tLogOut server\n");
    sendMessage(buffer, user);
}

// Manda la lista delle stanze e degli utenti all'utente che lo richiede
void helpListRooms(user_t *user)
{
    char msg[BUFFER_SIZE];
    logInfo("%s needs rooms", user->username);
    //printRoomList(roomList);

    sendMessage(MAGENTA "List rooms\n" RESET, user);

    pthread_mutex_lock(&roomList->mutex);
    room_t *room;
    for (room = roomList->head; room; room = room->next)
    {
        if (user->room == room)
        {
            snprintf(msg, sizeof(msg), CYAN "[%d - %s]" RESET " (%d users online) own by (%d - %s)\n",
                     room->uid, room->name, room->userList->count, room->owner->uid, room->owner->username);
        }
        else
        {
            snprintf(msg, sizeof(msg), "[%d - %s] (%d users online) own by (%d - %s)\n",
                     room->uid, room->name, room->userList->count, room->owner->uid, room->owner->username);
        }

        sendMessage(msg, user);
        helpListUsers(room, user);
    }

    pthread_mutex_unlock(&roomList->mutex);
}

// Manda la lista degli utenti di una stanza all'utente che la richiede
void helpListUsers(room_t *room, user_t *user)
{
    char       msg[BUFFER_SIZE];
    user_list *userList = room->userList;
    user_t *   tmp;
    pthread_mutex_lock(&userList->mutex);
    for (tmp = userList->head; tmp; tmp = tmp->next)
    {
        if (user == tmp)
        {
            snprintf(msg, sizeof(msg), CYAN "    %d - %s\n" RESET, tmp->uid, tmp->username);
        }
        else
        {
            snprintf(msg, sizeof(msg), "    %d - %s\n", tmp->uid, tmp->username);
        }
        sendMessage(msg, user);
    }

    pthread_mutex_unlock(&userList->mutex);
}

// Prompt per la creazione di una nuova stanza, si aspetta un Nome dal client
void helpNewRoom(user_t *user)
{
    char roomName[MAX_ROOMNAME_LENGTH] = {'\0'};
    int  receive;
    if (cleanEmptyRooms(roomList))
    {
        sendMessage(MAGENTA "Enter new room name:\n" RESET, user);
        receive = recv(user->sock_fd, roomName, BUFFER_SIZE, 0);
        strcpy(roomName, stripString(roomName));
        if (receive > 0 && strlen(roomName) > 0)
        {
            removeUserFromRoom(user);
            addRoom(roomList, createRoom(roomName, user));
        }
    }
    else
    {
        logWarn("Max room count reached!");
        sendMessage(RED "Max room count reached!\n" RESET, user);
    }
}

// Controlla se c'è spazio per altre stanze e se è possibile eliminare stanze vuote per creare spazio
// ritorna TRUE se c'è spazio, FALSE se non c'è spazio
int cleanEmptyRooms(room_list *roomList)
{
    if (roomList->roomCount < MAX_ROOM_COUNT)
    {
        logDebug("list not full");
        return 1; //if list not full
    }

    room_t *room;
    for (room = roomList->head->next; room; room = room->next)
    {
        if (room->userList->count == 0)
        {
            removeRoomFromList(roomList, room);
            logDebug("One room removed, now there's space");
            return TRUE; //if there at least one room empty
        }
    }
    logWarn("here's not space for another room");
    return FALSE; //if there's not space for another room
}

// selezione della nuova stanza, si aspetta un numero identificativo dal client
void helpChangeRoom(user_t *user)
{
    char buffer[BUFFER_SIZE] = {'\0'};
    int  roomUid;
    int  receive;
    sendMessage(MAGENTA "Select a room by number: \n" RESET, user);
    helpListRooms(user);
    receive = recv(user->sock_fd, buffer, BUFFER_SIZE, 0);
    strcpy(buffer, stripString(buffer));

    if (receive > 0 && strlen(buffer) > 0)
    {
        for (size_t i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] < '0' || buffer[i] > '9')
            {
                logWarn("UidRoom invalid : %s", buffer);
                sendMessage(RED "Please insert a valid number!\n" RESET, user);
                return;
            }
        }

        roomUid = atoi(buffer);

        room_t *room = selectRoom(roomList, roomUid);
        if (room == NULL)
        {
            bzero(buffer, sizeof(buffer));
            snprintf(buffer, BUFFER_SIZE, RED "Room with uid = %d not found\n" RESET, roomUid);
            sendMessage(buffer, user);
            return;
        }

        if (!changeUserRoom(user, room))
        {
            sendMessage(RED "You are already in this room!\n" RESET, user);
            return;
        }
    }
    else
    {
        logWarn("UidRoom invalid : %s", buffer);
        sendMessage(RED "Please insert a valid number!\n" RESET, user);
    }
}

// Elimina la stanza
void helpDeleteRoom(room_list *roomList, room_t *room, user_t *user)
{

    if (room != roomList->head && (room->owner == user || user->uid == 0))
    {
        sendMessage(RED "Deleting room...\n" RESET, user);
        sendBroadcastMessage(RED "Deleting room...\n" RESET, user);
        usleep(500);
        removeRoomFromList(roomList, room);
    }
    else
    {
        sendMessage(RED "401: Unouthorize\n" RESET, user);
        logError("(%d - %s) tried to delete room [%d - %s] without permission!",
                 user->uid, user->username, room->uid, room->name);
    }
}

// Ritorna TRUE (1) se l'admin è già loggato nel sistema, altrimenti FALSE (0)
int isAdminLogged(room_list *roomList)
{
    room_t *   room;
    user_t *   user;
    user_list *userList;
    pthread_mutex_lock(&roomList->mutex);
    for (room = roomList->head; room; room = room->next)
    {
        userList = room->userList;
        pthread_mutex_lock(&userList->mutex);
        for (user = room->userList->head; user; user = user->next)
        {
            if (user->uid == 0)
            {
                pthread_mutex_unlock(&userList->mutex);
                pthread_mutex_unlock(&roomList->mutex);
                return TRUE;
            }
        }
        pthread_mutex_unlock(&userList->mutex);
    }
    pthread_mutex_unlock(&roomList->mutex);
    return FALSE;
}
