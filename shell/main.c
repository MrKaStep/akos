#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "core.h"
#include "program.h"
#include "getters.h"

int _argc;
char** _argv;
pid_t current_foreground = -1;
pid_t* pids;
int g_cnt;

int handle_program(program* p)
{
    int err;
    err = execvp(p->name, p->arguments);
    fputs(p->name, stderr);
    perror("Can't execute program");
    return E_EXEC;
}

void ctrl_c_job(int signum)
{
    int i;
    int status;
    for(i = 0; i < g_cnt; ++i)
    {
        if(pids[i] != 0)
        {
            kill(pids[i], signum);
            waitpid(pids[i], &status, 0);
            fprintf(stderr, "Killed %d, pid: %d\n", i, pids[i]);
        }
    }
    exit(0);
}

int handle_job(job* j)
{
    int i;
    int** fd;
    if(strcmp(j->programs[0]->name, "exit") == 0)
    {
        job_destroy(j);
        destroy();
        exit(0);
    }
    fd = calloc(j->number_of_programs, sizeof(int*));
    for(i = 0; i j->number_of_programs; ++i)
    {
        fd[i] = calloc(2, sizeof(int));
    }
    pids = calloc(j->number_of_programs, sizeof(pid_t));
    for(i = 0; i < j->number_of_programs; ++i)
    {
        
    }
}

void ctrl_c_handler(int signum)
{
    if(current_foreground != -1)
    {
        kill(current_foreground, signum);
    }
    else
    {
        puts("");
    }
    signal(signum, ctrl_c_handler);
}

int main(int argc, char* argv[])
{
    _argc = argc;
    _argv = argv;
    init();
    signal(SIGINT, ctrl_c_handler);
    while(1)
    {
        int err;
        job* j;
        err = get_job(&j);
        if(err)
        {
            print_err(err, "Error getting job");
        }
        else
        {
            if(j->background)
            {
                pid_t pid = fork();
                if(pid == 0)
                {
                    close(0);
                    close(1);
                }
                else
                {
                    continue;
                }
            }
            handle_job(j);
        }
    }



    destroy();
    return 0;
}