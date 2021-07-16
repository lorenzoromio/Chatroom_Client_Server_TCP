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

#define _STR(x) #x
#define STR(x)  _STR(x)

#define CLEAR_TERMINAL      "\033[1;1H\033[2J"
#define CLEAR_LINE          "\33[2K"
#define CURSOR_UP           "\033[A"
#define MIN_USERNAME_LENGTH 3
#define MAX_USERNAME_LENGTH 25
#define MIN_ROOMNAME_LENGTH 3
#define MAX_ROOMNAME_LENGTH 25
#define BUFFER_SIZE         2048
#define MAX_MESSAGE_LENGTH  2000
#define TRUE                1
#define FALSE               0