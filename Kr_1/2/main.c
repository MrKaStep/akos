#define _XOPEN_SOURCE 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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
    E_OK,
    E_BADNUM,
    E_EOF
};

int64_t cur_ans = 0, cur_sum = 0;




void print_res()
{
    printf("%ld\n", cur_ans);
}



void upd_ans(int64_t a)
{
    int ans = 0;
    int i;
    for(i = 0; i < 54; ++i)
    {
        if((a & (511l << i)) == (455l << i))
            ++ans;
    }
    if(ans > cur_sum)
    {
        cur_sum = ans;
        cur_ans = a;
    }
}

int get_int(int64_t* a)
{
    char d[23];
    int c;
    size_t l = 0;
    while(isspace(c = getc(stdin)));
    if(c == EOF)
    {
        *a = 0;
        return E_OK;
    }
    if(c != '+' && c != '-' && !isdigit(c))
    {
        return E_BADNUM;
    }
    d[l++] = c;
    while(isdigit(c = getc(stdin)) && l < 21)
    {
        d[l++] = c;
    }
    *a = strtol(d, NULL, 10);
    if(errno == ERANGE)
        return E_BADNUM;
    return E_OK;
}

void ctrl_c_handler(int a)
{
    if(a == SIGINT)
    {
        printf("\nThe final result: ");
        print_res();
        exit(0);
    }
}

void handler(int a)
{
    if(a == SIGALRM)
    {
        printf("Current result: ");
        print_res();
        alarm(5);
    }
    signal(a, handler);
}

int main()
{
    signal(SIGALRM, handler);
    signal(SIGINT, ctrl_c_handler);
    alarm(5);
    while(1)
    {
        int64_t a;
        if(get_int(&a) == E_OK)
            upd_ans(a);
    }
    return 0;
}
