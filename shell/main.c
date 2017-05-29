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
pid_t* volatile pids;
int** volatile pipefd;
int **fd;
volatile int rem;
int cnt;

int handle_program(program* p)
{
    int err;
    if(strcmp(p->name, "cd") == 0)
    {
        return cd(p->number_of_arguments > 0 ? p->arguments[0] : home_dir);
    }
    err = execvp(p->name, p->arguments);
    fputs(p->name, stderr);
    perror("Can't execute program");
    return E_EXEC;
}

void chld_handler(int signum)
{
    pid_t pid;
    int status;
    int i;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for(i = 0; i < cnt; ++i)
        {
            if(pid == pids[i])
            {
                --rem;
                pids[i] = 0;
                break;
            }
        }
    }
    signal(signum, chld_handler);
}

void ctrl_c_job(int signum)
{
    int i;
    for(i = 0; i < cnt; ++i)
    {
        if(pids[i] != 0)
        {
            kill(pids[i], signum);
            waitpid(pids[i], NULL, 0);
        }
    }
    rem = 0;
    signal(signum, ctrl_c_job);
}

int handle_job(job* j)
{
    int i;
    pid_t pid_job = fork();
    if(strcmp(j->programs[0]->name, "exit") == 0)
    {
        job_destroy(j);
        destroy();
        exit(0);
    }
    if(pid_job == 0)
    {
        pids = calloc(j->number_of_programs, sizeof(pid_t));
        pipefd = calloc(j->number_of_programs - 1, sizeof(int*));
        fd = calloc(j->number_of_programs, sizeof(int*));
        signal(SIGINT, ctrl_c_job);
        signal(SIGCHLD, chld_handler);
        cnt = j->number_of_programs;
        rem = cnt;
        for(i = 0; i < j->number_of_programs - 1; ++i)
        {
            pipefd[i] = calloc(2, sizeof(int));
            pipe(pipefd[i]);
        }
        for(i = 0; i < j->number_of_programs; ++i)
        {
            int fd_in, fd_out, append;
            fd[i] = calloc(2, sizeof(int));
            fd[i][0] = (i == 0 ? 0 : pipefd[i - 1][0]);
            fd[i][1] = (i == j->number_of_programs - 1 ? 1 : pipefd[i][1]);
            if(j->programs[i]->input_file != NULL)
            {
                fd_in = open(j->programs[i]->input_file, O_RDONLY);
                if(fd_in == -1)
                {
                    perror("Can't read file");
                }
                else
                {
                    close(fd[i][0]);
                    fd[i][0] = fd_in;
                }
            }
            if(j->programs[i]->output_file != NULL)
            {
                append = (j->programs[i]->output_type == M_APPEND ? O_APPEND : O_TRUNC);
                fd_out = open(j->programs[i]->output_file, O_WRONLY | O_CREAT | append, 0644);
                if(fd_out == -1)
                {
                    perror("Can't write file");
                }
                else
                {
                    close(fd[i][1]);
                    fd[i][1] = fd_out;
                }
            }
        }
        for(i = 0; i < j->number_of_programs; ++i)
        {
            pids[i] = fork();
            if(pids[i] == 0)
            {
                fprintf(stderr, "Program %d, input = %d, output = %d\n", i, fd[i][0], fd[i][1]);
                dup2(fd[i][0], 0);
                dup2(fd[i][1], 1);
                handle_program(j->programs[i]);
            }
            else
            {
                fprintf(stderr, "Closing %d, %d\n", fd[i][0], fd[i][1]);
                close(fd[i][0]);
                close(fd[i][1]);
            }
        }
        while(rem);
        for(i = 0; i < j->number_of_programs; ++i)
        {
            free(fd[i]);
            if(i > 0)
                free(pipefd[i - 1]);
        }
        free(pids);
        free(pipefd);
        free(fd);
        job_destroy(j);
        destroy();
        exit(0);
    }
    else
    {
        int status;
        current_foreground = pid_job;
        waitpid(pid_job, &status, 0);
        signal(SIGCHLD, SIG_DFL);
        current_foreground = -1;
        job_destroy(j);
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