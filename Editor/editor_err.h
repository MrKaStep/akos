#ifndef EDITOR_ERR_H
#define EDITOR_ERR_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif /*_XOPEN_SOURCE*/

#include <stdio.h>
#include <wchar.h>

#define E_MALLOC 1
#define E_IOFAIL 2
#define E_STRRNG 3
#define E_LNRNG  4
#define E_C_WRNG 5
#define E_NOARG  6
#define E_NARROW 7
#define E_NOFILE 8
#define E_COMM   9
#define E_EMPTY  10
#define E_EOF    11
#define E_NOTSAV 12

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

int printerr(int err);

#endif /*EDITOR_ERR_H*/