#ifndef BUILT_IN_H
#define BUILT_IN_H

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>

#include "errors.h"
#include "core.h"

#define INT_CD   1
#define INT_PWD  2
#define INT_EXIT 3
#define INT_JOBS 4
#define INT_BG   5
#define INT_FG   6

int is_internal(char* name);

int cd(program* p);

int pwd();

void print_jobs();

#endif