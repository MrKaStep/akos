#ifndef ERRORS_H
#define ERRORS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include <stdio.h>
#include <unistd.h>

#define FUCK fputs("FUCK", stderr)

#define E_OK     0
#define E_MALLOC 1
#define E_USR    2
#define E_CHDIR  3
#define E_EOLN   4
#define E_REDIR  5

void print_err(int err);

#endif /*ERRORS_H*/