#ifndef CORE_H
#define CORE_H

#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "errors.h"
#include "utils.h"

#define PATH_MAX 4096 /*Временный костыль для непонятной ошибки*/

struct passwd* user;
char* username;
char* hostname;
char* home_dir;
char cur_path[PATH_MAX];
extern int _argc;
extern char** _argv;
int last_foreground_result;
pid_t shell_pid;

extern char* buf;
extern int need_invite;

void init_env();

int init();

void destroy();

#endif