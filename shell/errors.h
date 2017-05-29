#ifndef ERRORS_H
#define ERRORS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define FUCK fputs("FUCK\n", stderr)

#define E_OK      0
#define E_MALLOC -1
#define E_USR    -2
#define E_CHDIR  -3
#define E_REDIR  -5
#define E_QBREAK -6
#define E_UNEOLN -7
#define E_DOLLAR -8
#define E_ARGCOF -9
#define E_PIPERR -10
#define E_DREDIR -11
#define E_EXEC   -12

void print_err(int err, char* message);

#endif /*ERRORS_H*/