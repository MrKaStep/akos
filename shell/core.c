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
        printf("There are active or stopped jobs");
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