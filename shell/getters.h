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

void invite_user();

int get_command(char** buf);

void skip_spaces(char** buf);

int get_token(char** cur, char** dest);

int get_program(program* p);

int get_job(job** j);
#endif /*GETTERS_H*/