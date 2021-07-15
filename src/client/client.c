#include "client.h"
#include "../log/logger.h"
#include "../utils/common.h"
#include "../utils/utils.h"
#include "parser.h"

#include <ctype.h>
#include <netdb.h> //for resolveHostname
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h> //for number of colums
#include <termios.h>   //for getCurrentStdin

void  getCurrentStdin(char *prompt);
void *sendMessageHandler(void *arg);
void *recvMessageHandler(void *arg);
void  signalHandler(int sig);

// Variabili globali
int client_sd = 0;

typedef struct
{
    char *          buffer;
    char *          message;
    int             bufferSize;
    int             cursor;
    int             writingLine;
    pthread_mutex_t mutex;
} input;

input userInput = {NULL, NULL, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER};

int main(int argc, char **argv)
{
    char               username[MAX_USERNAME_LENGTH + 1];
    clientConfig       config;
    struct sockaddr_in server_addr;
    pthread_t          recv_msg_thread;
    pthread_t          send_msg_thread;
    int                option;
    int                status;


    config.host = CONNECTION_HOST;
    config.port = CONNECTION_PORT;
    parserArgv(argc, argv, &config);

    signal(SIGINT, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGQUIT, signalHandler);


    //increaseLogLevel();
    //increaseLogLevel();

    // Socket setting TCP
    client_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sd < 0)
    {
        logError("Socket creation failed");
        perror("");
        return EXIT_FAILURE;
    }

    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(config.host);
    server_addr.sin_port        = htons(config.port);

    // Imposto dimensione del buffer di ricezione e di invio
    option = BUFFER_SIZE;
    status = setsockopt(client_sd, SOL_SOCKET,
                        (SO_SNDBUF | SO_RCVBUF), (char *)&option, sizeof(option));

    logDebug("setsockopt status = %d", status);
    if (status < 0)
    {
        logError("setsockopt SO_SNDBUF | SO_RCVBUF failed");
        perror("");
        return EXIT_FAILURE;
    }

    // Connect to Server
    status = connect(client_sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (status < 0)
    {
        logError("Connection");
        perror("");
        return EXIT_FAILURE;
    }

    char buffer[1];
    int  receive = recv(client_sd, buffer, 1, 0);
    if (receive > 0)
    {
        logInfo("Connected");
    }
    else
    {
        logError("The server is full");
        exit(-1);
    }

    //printf(CLEAR_TERMINAL);
    //printf(YELLOW"=== WELCOME TO THE CHATROOM ===\n"RESET);

    char prompt[] = "Please enter your username (length: " STR(MIN_USERNAME_LENGTH) " - " STR(MAX_USERNAME_LENGTH) ")";

    while (1)
    {
        getCurrentStdin(prompt);
        strcpy(username, userInput.message);
        if (strlen(username) >= MIN_USERNAME_LENGTH && strlen(username) <= MAX_USERNAME_LENGTH)
        {
            break;
        }
        printf(CURSOR_UP CLEAR_LINE "\r"); //cursor up, erase line, carriage return
        logError("Username lenght invalid");
    }

    // Send username
    logDebug("Sending '%.*s'", MAX_USERNAME_LENGTH + 1, username);
    send(client_sd, username, MAX_USERNAME_LENGTH + 1, 0);

    printf(CLEAR_TERMINAL YELLOW "=== WELCOME TO THE CHATROOM ===\n" RESET);

    // Lancia i thread di gestione dei messaggi
    if (pthread_create(&send_msg_thread, NULL, &sendMessageHandler, (void *)username) != 0)
    {
        logError("pthread sendMessageHandler\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&recv_msg_thread, NULL, &recvMessageHandler, (void *)username) != 0)
    {
        logError("pthread recvMessageHandler\n");
        return EXIT_FAILURE;
    }

    // Attende che il thread finisca
    pthread_join(recv_msg_thread, NULL);
    logWarn("LOGOUT");
    close(client_sd);

    return EXIT_SUCCESS;
}

void signalHandler(int sig)
{
    switch (sig)
    {
    case 20:
        logWarn("CTRL-Z DETECTED");
        break;
    case 2:
        logWarn("CTRL-C DETECTED");
        break;

    default:
        logDebug("SIG %d DETECTED", sig);
        break;
    }

    close(client_sd);
    exit(sig);
}

// Gestore invio dei messaggi
void *sendMessageHandler(void *arg)
{
    char *username = (char *)arg;

    // questo rende stin immediato
    struct termios oldTeminal, newTeminal;
    tcgetattr(STDIN_FILENO, &oldTeminal);
    newTeminal = oldTeminal;
    newTeminal.c_lflag &= ~(ICANON);
    logDebug("Terminal set attributes %d", tcsetattr(STDIN_FILENO, TCSANOW, &newTeminal));

    while (1)
    {
        // READ INPUT
        getCurrentStdin(username);

        // IF NO MSG READY UNLOCK AND YIELD
        if (userInput.message == NULL)
        {
            sched_yield();
            continue;
        }
        // ELSE SEND MSG

        send(client_sd, userInput.message, strlen(userInput.message), 0);
        free(userInput.message);
        userInput.message = NULL;
    }

    signalHandler(0);

    return NULL;
}

// Gestore ricezione messaggi
void *recvMessageHandler(void *arg)
{
    char *         username             = (char *)arg;
    char           message[BUFFER_SIZE] = {'\0'};
    struct winsize w;
    while (1)
    {
        int receive = recv(client_sd, message, BUFFER_SIZE, 0);
        ioctl(0, TIOCGWINSZ, &w);
        if (receive > 0)
        {
            userInput.writingLine = (strlen(username) + 3 + userInput.cursor - 1) / w.ws_col;
            for (size_t i = 0; i < userInput.writingLine; i++)
            {
                printf("\r" CLEAR_LINE CURSOR_UP); //carriage return, erase line, cursor up,
            }

            printf(CLEAR_LINE "\r"
                              "%s",
                   message);

            printf(CLEAR_LINE "\r"
                              "%s > " CYAN "%.*s" RESET,
                   username, userInput.cursor, userInput.buffer);

            fflush(stdout);
        }
        else if (receive == 0)
        {
            break;
        }
        else
        {
            logError("errore");
            break;
        }
        bzero(message, sizeof(message));
    }

    return NULL;
}

void getCurrentStdin(char *prompt)
{
    char           key;
    struct winsize w;

    while (1)
    {
        ioctl(0, TIOCGWINSZ, &w);

        // printf ("lines %d\n", w.ws_row);
        // printf ("columns %d\n", w.ws_col);

        if (userInput.cursor == userInput.bufferSize)
        {
            userInput.bufferSize += 1024;
            userInput.buffer = realloc(userInput.buffer, userInput.bufferSize);
            logDebug("UserInput buffer grown: now at %p of size %d", userInput.buffer, userInput.bufferSize);
        }

        userInput.writingLine = (strlen(prompt) + 3 + userInput.cursor - 1) / w.ws_col;
        for (size_t i = 0; i < userInput.writingLine; i++)
        {
            printf("\r" CLEAR_LINE CURSOR_UP); //carriage return, erase line, cursor up,
        }

        printf(CLEAR_LINE "\r"
                          "%s > " CYAN "%.*s" RESET,
               prompt,
               userInput.cursor, userInput.buffer);

        fflush(stdout);

        // ignora le sequenze di escape
        key = getchar();
        switch (key)
        {
        case 9: // tab
            continue;
            break;
        case 27:       // escape
            getchar(); //91
            key = getchar();
            if (key == 50 || key == 51 || key == 53 || key == 54) // freccie
            {
                getchar(); //tilde
            }
            continue;
        default:
            break;
        }

        userInput.buffer[userInput.cursor] = key;

        // msg end
        if (userInput.buffer[userInput.cursor] == '\n')
        {
            /*
            // Cancella il tuo input
            printf(CURSOR_UP);
            for (size_t i = 0; i < userInput.writingLine; i++)
            {
                printf(CURSOR_UP CLEAR_LINE "\r"); //carriage return, erase line, cursor up,
            }
            */

            userInput.buffer[userInput.cursor] = '\0';
            userInput.message                  = strdup(stripString(userInput.buffer));
            userInput.cursor                   = 0;
            userInput.writingLine              = 0;
            break;
        }
        // backspace
        else if (userInput.buffer[userInput.cursor] == '\b' || userInput.buffer[userInput.cursor] == 127)
        {
            if (userInput.cursor > 0)
            {
                --userInput.cursor;
                if (((strlen(prompt) + 2 + userInput.cursor) / w.ws_col) < userInput.writingLine)
                {
                    printf("\r" CLEAR_LINE CURSOR_UP);
                }
            }
        }
        else if (userInput.buffer[userInput.cursor] == EOF)
        {
            break;
        }
        else if (userInput.cursor < MAX_MESSAGE_LENGTH)
        {
            ++userInput.cursor;
        }
    }
}

// Traduce un hostname in un indirizzo ip
int resolveHostname(char *hostname, char *ip)
{
    struct hostent * host;
    struct in_addr **addr_list;

    host = gethostbyname(hostname);
    if (host == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **)host->h_addr_list;

    for (size_t i = 0; addr_list[i] != NULL; i++)
    {
        // Ritorna il primo indirizzo valido
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 1;
}