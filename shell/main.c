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
#include <errno.h>

#include "core.h"
#include "program.h"
#include "getters.h"
#include "built_in.h"






void wait_job(job* j)
{
    int status;
    waitpid(j->pid, &status, WUNTRACED);
    if(WIFSTOPPED(status))
    {
        j->stopped = 1;
        print_job(j);
    }
    else
    {
        j->stopped = 0;
        j->complete = 1;
        jobs_alive -= 1;
        last_foreground_result = WEXITSTATUS(status);
    }
}

void make_fg(job* j, int to_continue)
{
    struct termios old_termios;
    int status;
    j->stopped = 0;
    j->complete = 0;
    tcgetattr(shell_terminal, &old_termios);
    tcsetpgrp(shell_terminal, j->pid);
    if(to_continue)
    {
        kill(-j->pid, SIGCONT);
    }
    wait_job(j);
    tcsetpgrp(shell_terminal, shell_pid);
}

void make_bg(job* j, int to_continue)
{
    j->stopped = j->complete = 0;
    if(to_continue)
    {
        kill(-j->pid, SIGCONT);
    }
}

void handle_internal(program* p, char piped)
{
    int c = is_internal(p->name);
    switch(c)
    {
    case INT_CD:
        cd(p);
        break;
    case INT_PWD:
        pwd();
        break;
    case INT_EXIT:
        inv_exit(0);
        break;
    case INT_JOBS:
        print_jobs();
        break;
    case INT_BG:
        if(piped)
        {
            printf("bg: no effect when piped\n");
        }
        else
        {
            int i;
            pid_t pid;
            pid = (pid_t)strtol(
                      p->arguments[1],
                      p->arguments[1] + strlen(p->arguments[1]),
                      10
                  );
            for(i = 0; i < jobs_count; ++i)
            {
                if(jobs[i]->stopped && jobs[i]->pid == pid)
                {
                    make_bg(jobs[i], 1);
                    return;
                }
            }
        }
        break;
    case INT_FG:
        if(piped)
        {
            int i;
            printf("fg: no effect when piped\n");
        }
        else
        {
            int i;
            pid_t pid;
            pid = (pid_t)strtol(
                      p->arguments[1],
                      p->arguments[1] + strlen(p->arguments[1]),
                      10
                  );
            for(i = 0; i < jobs_count; ++i)
            {
                if(jobs[i]->stopped && jobs[i]->pid == pid)
                {
                    make_fg(jobs[i], 1);
                    return;
                }
            }
        }
        break;
    }
    if(piped)
    {
        inv_exit(1);
    }
}

int handle_program(program* p, int inp, int out)
{
    int fd_in, fd_out;
    if(p->input_file != NULL)
    {
        fd_in = open(p->input_file, O_RDONLY);
        if(fd_in == -1)
        {
            perror("Can't read file");
            inv_exit(1);
        }
        if(inp != STDIN_FILENO)
            close(inp);
        inp = fd_in;
    }
    if(p->output_file != NULL)
    {
        fd_out = open(p->output_file, O_WRONLY | O_CREAT | (p->output_type == M_APPEND ? O_APPEND : 0));
        if(fd_out == -1)
        {
            perror("Can't write to file");
            inv_exit(1);
        }
        if(out != STDOUT_FILENO)
            close(out);
        out = fd_out;
    }
    if(inp != STDIN_FILENO)
    {
        dup2(inp, STDIN_FILENO);
        close(inp);
    }
    if(out != STDOUT_FILENO)
    {
        dup2(out, STDOUT_FILENO);
        close(out);
    }
    if(is_internal(p->name))
    {
        handle_internal(p, 1);
    }
    execvp(p->name, p->arguments);
    perror("exec");
    exit(1);
}

int handle_job(job* j)
{
    int i;
    pid_t pid;
    int inp = STDIN_FILENO, out = STDOUT_FILENO;

    j->complete = 0;
    j->stopped = 0;

    pid = fork();

    if(pid == 0)
    {
        int pipefd[2];
        int i;
        int status;

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        setpgid(getpid(), getpid());
        for(i = 0; i < j->number_of_programs; ++i)
        {
            if(i != j->number_of_programs - 1)
            {
                pipe(pipefd);
                out = pipefd[1];
            }
            else
            {
                out = STDOUT_FILENO;
            }
            pid = fork();
            if(pid == 0)
            {
                handle_program(j->programs[i], inp, out);
            }
            if(i != j->number_of_programs - 1)
                inp = pipefd[0];
        }
        do
        {
            errno = 0;
        }
        while(wait(&status) != -1 && errno != ECHILD);
        inv_exit(1);
    }
    else
    {
        int status;
        if(jobs_capacity == jobs_count)
        {
            jobs_capacity += expand_array((void**)&jobs, jobs_capacity, sizeof(job*));
        }
        jobs[jobs_count++] = j;
        ++jobs_alive;
        j->pid = pid;
        if(!j->background)
        {
            make_fg(j, 0);
        }
        else
        {
            make_bg(j, 0);
        }
    }
}


int main(int argc, char* argv[])
{
    _argc = argc;
    _argv = argv;
    init();
    while(1)
    {
        job* j;
        if(get_job(&j) == E_UNEOLN)
        {
            puts("");
            inv_exit(0);
        }
        else
        {
            if(j->number_of_programs == 0)
            {
                job_destroy(j);
                continue;
            }
            if(j->number_of_programs == 1)
            {
                if(is_internal(j->programs[0]->name))
                {
                    handle_internal(j->programs[0], 0);
                    job_destroy(j);
                    continue;
                }
            }
            handle_job(j);
        }
    }


    inv_exit(1);
    return 0;
}