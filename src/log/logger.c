/*
 * Copyright (c) 2021 <lukaarma@gmail.com>
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

#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

logLevel loggerLevel = INFO;

void logger(logLevel level, char *file, const char *func, int line, char *fmt, ...)
{
    if (level <= loggerLevel)
    {
        va_list args;
        va_start(args, fmt);

        switch (level)
        {
        case DEBUG:
            printf(MAGENTA "[DEBUG] " RESET);

            break;

        case VERBOSE:
            printf(CYAN "[VERBOSE] " RESET);

            break;

        case INFO:
            printf(GREEN "[INFO] " RESET);

            break;

        case WARNING:
            printf(YELLOW "[WARNING] " RESET);

            break;

        case ERROR:
            printf(RED "[ERROR] " RESET);

            break;

        case PANIC:
            printf(RED "[FATAL ERROR] " GRAY "%s:%d [%s] " RESET, file, line, func);
            vprintf(fmt, args);
            printf("\n");

            exit(1);

        default:
            panic("Invalid logger level! \n");

            break;
        }

        if (loggerLevel >= DEBUG)
        {
            printf(GRAY "%s:%d [%s] " RESET, file, line, func);
        }

        vprintf(fmt, args);
        printf("\n");

        va_end(args);
    }
}

void logFile(const char *file, int overwrite, char *fmt, ...)
{
    FILE *fp;
    va_list args;

    if (overwrite)
    {
        fp = fopen(file, "w+");
    }
    else
    {
        fp = fopen(file, "a+");
    }

    if (fp == NULL)
    {
        logError("Could not open '%s' to write!", file);
    }

    va_start(args, fmt);
    vfprintf(fp, fmt, args);

    fclose(fp);
}

void increaseLogLevel()
{
    if (loggerLevel < DEBUG)
    {
        ++loggerLevel;
    }
}
