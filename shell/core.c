#include "core.h"

void init_env()
{
    setenv("SHELL", _argv[0], 1);
}

int init()
{
    uid_t uid = getuid();
    user = getpwuid(uid);
    if(user == NULL)
    {
        perror("User unrecognised");
        return E_USR;
    }
    username = user->pw_name;
    home_dir = user->pw_dir;
    hostname = calloc(64, sizeof(char));
    if(hostname == NULL)
    {
        print_err(E_MALLOC, "Cant get hostname");
    }
    if(gethostname(hostname, 64) == -1)
    {
        perror("Hostname failure");
    }
    getcwd(cur_path, PATH_MAX);
    last_foreground_result = 0;
    shell_pid = getpid();
    buf = NULL;
    need_invite = 1;
    return E_OK;
}

void destroy()
{
    free(hostname);
    free(buf);
}