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
