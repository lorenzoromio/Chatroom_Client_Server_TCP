#include "../log/logger.h"
#include "../utils/common.h"
#include "client.h"
#include <argp.h>
#include <string.h>
#include <stdlib.h>

error_t parserOpzioni(int key, char *arg, struct argp_state *state)
{
    clientConfig *config = state->input;
    char ip[45]; //Lunghezza massima iPv6


    switch (key)
    {
    case 'h':
        
        config->host = strdup(arg);
        logDebug("Host impostato '%s'!", config->host);
        break;

    case 'd':
        
        resolveHostname(arg ,ip);
        config->host = strdup(ip);
        logDebug("Dominio impostato '%s'! \n", config->host);
        logInfo("Host tradotto '%s'! \n", ip);
        break;

    case 'p':
        config->port = atoi(arg);
        logDebug("Porta impostata '%d'! \n", config->port);

        break;

    case ARGP_KEY_ARG:
        logDebug("Questa '%s' non è un opzione, viene vista come argomento! \n", arg);

        break;

    case ARGP_KEY_END:
        logDebug("Processati %d arguments \n", state->arg_num);

        break;

    default:

        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

void parserArgv(int argc, char **argv, clientConfig *config)
{
    struct argp_option options[] = {
        {"port", 'p', "PORT", 0, "Il client si connetterà su questa porta (default "STR(CONNECTION_PORT)")"},
        {"host", 'h', "IP", 0, "Il client si connetterà a questo indirizzo (default "CONNECTION_HOST")"},
        {"domain", 'd', "DOMAIN", 0, "Il server si connetterà a questo host (default localhost)"},
        {0}};

    struct argp argp = {options, parserOpzioni};

    argp_parse(&argp, argc, argv, 0, 0, config);
}
