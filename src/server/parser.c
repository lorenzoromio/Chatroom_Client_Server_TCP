#include "../log/logger.h"
#include "../utils/common.h"
#include "server.h"
#include <argp.h>
#include <stdlib.h>
#include <string.h>

error_t parserOpzioni(int key, char *arg, struct argp_state *state)
{
    serverConfig *config = state->input;

    switch (key)
    {
    case 'h':

        config->host = strdup(arg);
        logDebug("Host impostato '%s'!", config->host);
        break;

    case 'p':

        config->port = atoi(arg);
        logDebug("Porta impostata '%d'!", config->port);
        break;

    case ARGP_KEY_ARG:
        logDebug("Questa '%s' non è un opzione, viene vista come argomento!", arg);

        break;

    case ARGP_KEY_END:
        logDebug("Processati %d arguments", state->arg_num);

        break;

    default:

        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

void parserArgv(int argc, char **argv, serverConfig *config)
{
    struct argp_option options[] = {
        {"port", 'p', "PORT", 0, "Il server ascolterà su questa porta (default " STR(SERVER_PORT) ")"},
        {"host", 'h', "IP", 0, "Il server ascolterà  a questo indirizzo (default " SERVER_HOST ")"},
        {0}};

    struct argp argp = {options, parserOpzioni};

    argp_parse(&argp, argc, argv, 0, 0, config);
}
