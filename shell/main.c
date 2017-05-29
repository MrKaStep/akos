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

int handle_program(program* p)
{
    int err;
    err = execvp(p->name, p->arguments);
    fputs(p->name, stderr);
    perror("Can't execute program");
    return E_EXEC;
}

void handle_job(job* j)
{
    int i;
    int pipefd[2][2];
    signal(SIGCHLD, SIG_IGN);
    for(i = 0; i < j->number_of_programs; ++i)
    {
        pid_t pid;
        int fd_in = 0, fd_out = 1;
        if(strcmp(j->programs[i]->name, "exit") == 0)
        {
            job_destroy(j);
            destroy();
            exit(0);
        }
        if(i + 1 < j->number_of_programs)
        {
            pipe(pipefd[i & 1]);
        }
        if(i > 0)
        {
            fd_in = pipefd[(i & 1) ^ 1][0];
        }
        if(i + 1 < j->number_of_programs)
        {
            fd_out = pipefd[i & 1][1];
        }
        if(j->programs[i]->input_file != NULL)
        {
            int fd = open(j->programs[i]->input_file, O_RDONLY);
            if(fd == -1)
            {
                perror("Can't open file to read");
            }
            else
            {
                if(fd_in != 0)
                    close(fd_in);
                fd_in = fd;
            }
        }
        if(j->programs[i]->output_file != NULL)
        {
            int append = j->programs[i]->output_type == M_APPEND ? O_APPEND : O_TRUNC;
            int fd = open(j->programs[i]->output_file, O_WRONLY | O_CREAT | append, 0644);
            if(fd == -1)
            {
                perror("Can't open file to read");
            }
            else
            {
                if(fd_out != 1)
                    close(fd_out);
                fd_out = fd;
            }
        }
        pid = fork();
        if(pid == 0)
        {
            dup2(fd_in, 0);
            dup2(fd_out, 1);
            if(handle_program(j->programs[i]))
            {
                exit(E_EXEC);
            }
        }
        else
        {
            int status;
            current_foreground = pid;
            waitpid(pid, &status, 0);
            current_foreground = -1;
            if(i > 0 || j->programs[i]->input_file != NULL)
                close(fd_in);
            if(i + 1 < j->number_of_programs || j->programs[i]->output_file != NULL)
                close(fd_out);
            if(WIFEXITED(status))
            {
                if(!j->background)
                {
                    last_foreground_result = WEXITSTATUS(status);
                }
                if(WEXITSTATUS(status) != 0)
                {
                    fprintf(stderr, "Process %d exited with non-zero code %d\n", pid, WEXITSTATUS(status));
                }
            }
            else if(WIFSIGNALED(status))
            {
                fprintf(stderr, "Process %d was terminated by a signal %d\n", pid, WTERMSIG(status));
            }
        }
    }
    job_destroy(j);
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