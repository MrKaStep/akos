#define _XOPEN_SOURCE 1000
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>

enum ERR
{
    E_NOARG,
    E_PIPE,
    E_EXEC,
    E_FORK,
    E_OK
};

int main(int argc, char* argv[])
{
    pid_t pid;
    int fd[2];
    int f;
    if(argc != 6)
    {
        fputs("\tNeed 5 arguments", stderr);
    }
    if(pipe2(fd, O_CLOEXEC) != 0)
    {
        perror("Pipe creating error occured: ");
        return E_PIPE;
    }
    if(dup2(fd[1], 1) == -1)
    {
        perror("Pipe creating error occured: ");
        return E_PIPE;        
    }
    pid = fork();
    if(pid == -1)
    {
        perror("Fork failed");
        return E_FORK;
    }
    if(pid == 0)
    {
        if(execlp(argv[1], argv[1], "-h", NULL))
        {
            perror("Exec failed: ");
            return E_EXEC;
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if(!WIFEXITED(status))
        {
            fprintf(stderr, "Process don't stop\n");
            return E_FORK;
        }
    }
    pid = fork();
    if(pid == -1)
    {
        perror("Fork failed");
        return E_FORK;
    }
    if(pid == 0)
    {
        if(execlp(argv[2], argv[2], "-v", NULL))
        {
            perror("Exec failed: ");
            return E_EXEC;
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if(!WIFEXITED(status))
        {
            fprintf(stderr, "Process don't stop\n");
            return E_FORK;
        }
    }
    pid = fork();
    if(pid == -1)
    {
        perror("Fork failed");
        return E_FORK;
    }
    if(pid == 0)
    {
        if(execlp(argv[3], argv[3], "-f", argv[5], NULL))
        {
            perror("Exec failed: ");
            return E_EXEC;
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if(!WIFEXITED(status))
        {
            fprintf(stderr, "Process don't stop\n");
            return E_FORK;
        }
    }
    dup2(fd[0], 0);
    f = open(argv[5], O_CREAT | O_APPEND | O_WRONLY, 0666);
    dup2(1, f);
    pid = fork();
    if(pid == -1)
    {
        perror("Fork failed");
        return E_FORK;
    }
    if(pid == 0)
    {
        if(execlp(argv[4], argv[4], NULL))
        {
            perror("Exec failed: ");
            return E_EXEC;
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if(!WIFEXITED(status))
        {
            fprintf(stderr, "Process don't stop\n");
            return E_FORK;
        }
        if(WEXITSTATUS(status) != 0)
        {
            return 0;
        }
    }
    puts("Ok");
    return 0;

}