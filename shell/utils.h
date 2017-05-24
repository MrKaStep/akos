#ifndef UTILS_H
#define UTILS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "core.h"

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"


void set_color(char* color);

void print_path(int full_path);

size_t expand_array(void **s, size_t len, size_t item);

#endif /*UTILS_H*/