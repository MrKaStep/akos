#include "core.h"

void init_env()
{
    struct passwd* user;
    char s_pid[16];

    user = getpwuid(getuid());
    setenv("HOME", user->pw_dir, 1);
    setenv("USERNAME", user->pw_name, 1);

    setenv("SHELL", _argv[0], 1);

    sprintf(s_pid, "%d", (int)getpid());
    setenv("PID", s_pid, 1);
}

int init()
{
    shell_pid = getpid();
    shell_terminal = STDIN_FILENO;
    setpgid(shell_pid, shell_pid);

    init_env();
    gethostname(hostname, 4096);
    buf = NULL;

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    tcsetpgrp(shell_terminal, shell_pid);
    tcgetattr(shell_terminal, &shell_termios);

    jobs = calloc(1, sizeof(job*));
    if(jobs == NULL)
    {
        print_err(E_MALLOC, "init");
        exit(E_MALLOC);
    }
    jobs_capacity = 1;
    jobs_count = 0;
    jobs_alive = 0;

    need_invite = 1;

    return E_OK;
}

int inv_exit(char force)
{
    size_t i;
    if(!force && jobs_alive > 0)
    {
        printf("There are active or stopped jobs\n");
        return 0;
    }
    for(i = 0; i < jobs_count; ++i)
    {
        job_destroy(jobs[i]);
    }
    free(jobs);
    free(buf);
    if(getpid() == shell_pid)
        printf("exit\n");
    exit(0);
}






void wait_job(job* j)
{
    int status;
    waitpid(j->pid, &status, WUNTRACED);
    if(WIFSTOPPED(status))
    {
        j->stopped = 1;
        j->complete = 0;
        print_job(j);
    }
    else
    {
        j->stopped = 0;
        j->complete = 1;
        j->printed = 1;
        jobs_alive -= 1;
        last_foreground_result = WEXITSTATUS(status);
    }
}

void make_fg(job* j, int to_continue)
{
    struct termios old_termios;
    j->stopped = 0;
    j->complete = 0;
    j->printed = 1;
    if(to_continue)
        print_job_call(j);
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
    print_job(j);
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
        inv_exit(piped);
        break;
    case INT_JOBS:
        print_jobs();
        break;
    default:
        if(piped)
        {
            printf("%s: no effect when piped\n", c == INT_BG ? "bg" : "fg");
        }
        else
        {
            size_t i;
            pid_t pid;
            char* end;
            if(p->number_of_arguments > 1)
            {
                pid = (pid_t)strtol(
                          p->arguments[1],
                          &end,
                          10
                      );
                for(i = 0; i < jobs_count; ++i)
                {
                    if(jobs[i]->stopped && jobs[i]->pid == pid)
                    {
                        if(c == INT_FG)
                            make_fg(jobs[i], 1);
                        else
                            make_bg(jobs[i], 1);
                        return;
                    }
                }
            }
            else
            {
                job* j = NULL;
                for(i = 0; i < jobs_count; ++i)
                {
                    if(jobs[i]->stopped)
                        j = jobs[i];
                }
                if(j != NULL)
                {
                    if(c == INT_BG)
                        make_bg(j, 1);
                    else
                        make_fg(j, 1);
                    return;
                }
            }
            printf("%s: no such job\n", c == INT_BG ? "bg" : "fg");
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
            if(inp != STDIN_FILENO)
                close(inp);
            if(out != STDOUT_FILENO)
                close(out);
            if(i != j->number_of_programs - 1)
                inp = pipefd[0];
        }
        for(i = 0; i < j->number_of_programs; ++i)
        {
            pid = waitpid(-1, &status, WUNTRACED);
            status = WEXITSTATUS(status);
        }
        job_destroy(j);
        inv_exit(1);
    }
    else
    {
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
    return 0;
}

void update()
{
    size_t i;
    for(i = 0; i < jobs_count; ++i)
    {
        pid_t pid;
        if(!jobs[i]->complete && !jobs[i]->stopped)
        {
            int status;
            pid = waitpid(jobs[i]->pid, &status, WNOHANG | WUNTRACED);
            if(pid)
            {
                if(WIFSTOPPED(status))
                {
                    jobs[i]->complete = 0;
                    jobs[i]->stopped = 1;
                    jobs[i]->printed = 0;
                    print_job(jobs[i]);
                }
                else
                {
                    jobs[i]->stopped = 0;
                    jobs[i]->complete = 1;
                    --jobs_alive;
                    print_job(jobs[i]);
                    jobs[i]->printed = 1;
                }
            }
        }
    }
}