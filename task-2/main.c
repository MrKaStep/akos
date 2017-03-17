#define _XOPEN_SOURCE 1000

#include <linux/limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define E_UNKNWN 1000
#define E_NOARG  1001
#define E_NOTDIR 1002
#define E_MALLOC 1003
#define E_BADMD  1004
#define E_ACCES  1005


#define USE_STAT  256
#define USE_LSTAT 257

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
        return E_BADMD;
    }
    return (int)S_ISDIR(st.st_mode);
}

int is_link(const char* path, int mode)
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
        return E_BADMD;
    }
    return (int)S_ISLNK(st.st_mode);
}


int copy(const char* source, const char* dest);

void copy_mode(const char* source, const char* dest)
{
    struct stat st;
    lstat(source, &st);
    chmod(dest, st.st_mode);
}

int copy_dir(const char* source, const char* dest)
{
    DIR* d;
    DIR* s;
    struct dirent *f;
    int dl;
    int sl;
    char* new_s;
    char* new_d;
    mkdir(dest, 0777);
    if(!is_dir(dest, USE_LSTAT))
    {
        return E_NOTDIR;
    }
    d = opendir(dest);
    if(d == NULL)
        if(errno == EACCES)
            return E_ACCES;
        else
            return E_UNKNWN;
    s = opendir(source);
    if(s == NULL)
        if(errno == EACCES)
            return E_ACCES;
        else
            return E_UNKNWN;
    copy_mode(source, dest);
    dl = strlen(dest);
    sl = strlen(source);
    new_s = malloc((257 + sl) * sizeof(char));
    if(new_s == NULL)
        return E_MALLOC;
    new_d = malloc((257 + dl) * sizeof(char));
    if(new_d == NULL)
        return E_MALLOC;
    strcpy(new_s, source);
    strcpy(new_d, dest);
    while((f = readdir(d)) != NULL)
    {
        int ret;
        sprintf(new_s + sl, "/%s", f->d_name);
        sprintf(new_d + dl, "/%s", f->d_name);
        if((ret = copy(new_s, new_d)) != 0 )
        {
            free(new_s);
            free(new_d);
            return ret;
        }
    }
    closedir(d);
    closedir(s);
    free(new_s);
    free(new_d);
    return 0;
}

int copy_file(const char* source, const char* dest)
{
    FILE* s;
    FILE* d;
    d = fopen(dest, "wb");
    if(d == NULL)
        if(errno == EACCES)
            return E_ACCES;
        else
            return E_UNKNWN;
    s = fopen(source, "rb");
    if(s == NULL)
        if(errno == EACCES)
            return E_ACCES;
        else
            return E_UNKNWN;
    void* buf = malloc(8);
    while(!feof(s))
    {
        int n = fread(buf, 1, 8, s);
        fwrite(buf, 1, n, d);
    }
    copy_mode(source, dest);
    free(buf);
    fclose(s);
    fclose(d);
    return 0;
}

int copy_link(const char* source, const char* dest)
{
    char* path = malloc(PATH_MAX * sizeof(char));
    int t;
    if(readlink(source, path, PATH_MAX), t = errno)
    {
        printf("read %d\n", t);
        if(errno == EACCES)
            return E_ACCES;
        if(errno == ENOTDIR)
            return E_NOTDIR;
        return E_UNKNWN;
    }
    if(symlink(path, dest))
    {
        printf("sym %d\n", errno);
        if(errno == EACCES)
            return E_ACCES;
        if(errno == ENOTDIR)
            return E_NOTDIR;
        return E_UNKNWN;
    }
    copy_mode(source, dest);
    return 0;
}

int copy(const char* source, const char* dest)
{
    
    return 0;
}

int print_err(int err)
{
    switch(err)
    {
    case 0:
        return 0;
    case E_NOARG:
        fputs("Not enough arguments", stderr);
        break;
    case E_NOTDIR:
        fputs("Given path is not a directory", stderr);
        break;
    case E_MALLOC:
        fputs("Not enough memory", stderr);
        break;
    case E_BADMD:
        fputs("Incorrect mode for is_dir", stderr);
        break;
    case E_ACCES:
        fputs("Permission denied", stderr);
        break;
    case E_UNKNWN:
        fputs("Unknown error", stderr);
        break;
    default:
        fputs("Fucking unknown error", stderr);
        break;
    }
    return err;
}

int main(int argc, char const *argv[])
{
    char* dest;

    if(argc < 3)
    {
        fputs("Not enough arguments", stderr);
        return E_NOARG;
    }
    return print_err(copy_link(argv[1], argv[2]));
}