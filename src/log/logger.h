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

#pragma once

//TEXT COLOR

#define RESET   "\x1b[0m"
#define BLACK   "\x1b[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define GRAY    "\x1b[90m"

//BACKGROUND COLOR
#define BACK_BLACK   "\x1b[40m"
#define BACK_RED     "\x1b[41m"
#define BACK_GREEN   "\x1b[42m"
#define BACK_YELLOW  "\x1b[43m"
#define BACK_BLUE    "\x1b[44m"
#define BACK_MAGENTA "\x1b[45m"
#define BACK_CYAN    "\x1b[46m"
#define BACK_WHITE   "\x1b[47"

#define logDebug(fmt, ...)   logger(DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define logVerbose(fmt, ...) logger(VERBOSE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...)    logger(INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define logWarn(fmt, ...)    logger(WARNING, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define logError(fmt, ...)   logger(ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define panic(fmt, ...)      logger(PANIC, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum
{
    PANIC,
    ERROR,
    WARNING,
    INFO,
    VERBOSE,
    DEBUG
} logLevel;

void logger(logLevel level, char *file, const char *func, int line, char *fmt, ...);
void logFile(const char *file, int overwrite, char *fmt, ...);

void increaseLogLevel();
