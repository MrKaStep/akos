#ifndef CORE_H
#define CORE_H

#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <termios.h>
#include <signal.h>


#include "errors.h"
#include "utils.h"
#include "job.h"
#include "built_in.h"

#define PATH_MAX 4096 /*Временный костыль для непонятной ошибки*/

int _argc;
char** _argv;

pid_t shell_pid;
struct termios shell_termios;
int shell_terminal;

char hostname[4096];

extern char* buf;

int last_foreground_result;

job** jobs;
job* current_job;

size_t jobs_capacity, jobs_count, jobs_alive;

extern int need_invite;


void init_env();

int init();

int inv_exit(char force);

void wait_job(job* j);

void make_fg(job* j, int to_continue);

void make_bg(job* j, int to_continue);

void handle_internal(program* p, char piped);

int handle_program(program* p, int inp, int out);

int handle_job(job* j);

void update();




#endif