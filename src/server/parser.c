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
