#ifndef GETTERS_H
#define GETTERS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "job.h"
#include "program.h"
#include "errors.h"

extern struct passwd* user;
extern char* username;
extern char* hostname;
extern char* home_dir;
extern char cur_path[PATH_MAX];

char* buf;
char* cur;
size_t buf_size;

void invite_user();

int get_command();

void skip_spaces();

int get_token(char** dest);

int get_program(program** p);

int get_job(job** j);
#endif /*GETTERS_H*/