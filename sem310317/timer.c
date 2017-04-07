#define _XOPEN_SOURCE 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>

#define PID_FILE "/var/run/my_time_ls"

enum ERR
{
    E_NOARG,
    E_NOUSR,
    E_NOTDIR,
    E_BADMD,
    E_IOFAIL,
    E_LOCK,
    E_FORK
};

#define USE_STAT  256
#define USE_LSTAT 257

char buf[1024]; 

int is_dir(const char* path, int mode)
{
    struct stat st;
    switch(mode)
    {
    case USE_STAT:
        stat(path, &st);
        break;
    case USE_LSTAT:
        lstat(path, &st);
        break;
    default:
        exit(E_BADMD);

    }
    return (int)((st.st_mode & S_IFMT) == S_IFDIR);
}

int cnt = 0, timeout = 1;

void handler(int a)
{
    if(a == SIGTERM)
    {
        exit(0);
    }
    syslog(LOG_NOTICE, "%d", ++cnt);
    signal(a, handler);
    alarm(timeout);
}

int main(int argc, char* argv[])
{
    char* user;
    char* path;
    struct passwd* user_info = NULL;
    pid_t pid;
    gid_t gid;
    uid_t uid;
    int lock_fd;
    if(argc < 4)
    {
        fprintf(stderr, "\tNot enough arguments\n");
        return E_NOARG;
    }
    user = argv[1];
    path = argv[2];
    timeout = atoi(argv[3]);
    if(!is_dir(path, USE_LSTAT))
    {
        fprintf(stderr, "\t'%s' is not a directory\n", path);
        return E_NOTDIR;
    }
    user_info = getpwnam(user);
    if(user_info == NULL)
    {
        fprintf(stderr, "\t'%s' is not a user on this machine\n", user);
        return E_NOUSR;
    }
    pid = fork();
    if(pid == -1)
    {
        fprintf(stderr, "\tFork failed\n");
        return E_FORK;
    }
    close(0);
    close(1);
    close(2);
    if(pid != 0)
    {
        return 0;
    }
    lock_fd = open(PID_FILE, O_WRONLY | O_CREAT, 0644);
    if(lock_fd == -1)
    {
        fprintf(stderr, "\tFile '%s' opening failed: %s\n", PID_FILE, strerror(errno));
        return E_IOFAIL;
    }
    if(flock(lock_fd, LOCK_EX))
    {
        fprintf(stderr, "\tLock failed: %s\n", strerror(errno));
        return E_LOCK;
    }
    pid = getpid();
    sprintf(buf, "%d", pid);
    write(lock_fd, buf, strlen(buf) * sizeof(char));
    /**********/
    setpgid(0, 0);
    setsid();
    setuid(user_info->pw_uid);
    setgid(user_info->pw_gid);
    setegid(user_info->pw_gid);
    seteuid(user_info->pw_uid);
    openlog("time_ls", LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_NOTICE, "Timer ls started successfully: uid=%d, gid=%d, euid=%d, egid=%d", 
           (int)getuid(), (int)getgid(), (int)geteuid(), (int)getegid());
    signal(SIGALRM, handler);
    alarm(timeout);
    while(1)
        pause();    
}











