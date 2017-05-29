#ifndef GETTERS_H
#define GETTERS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <ctype.h>

#include "core.h"
#include "utils.h"
#include "job.h"
#include "program.h"
#include "errors.h"

#define M_NONE  0
#define M_QUOT  1
#define M_DQUOT 2

char* buf;
char* cur;
size_t buf_size;
int need_invite;

void invite_user();

char spec_symb(char c);

int get_command();

void skip_spaces();

int get_var(char** dest);

int get_token(char** dest, int* md);

int get_program(program** p);

int get_job(job** j);
#endif /*GETTERS_H*/