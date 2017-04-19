#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define E_OK     0
#define E_MALLOC 1
#define E_CORPT  2
#define E_EOF    3

int err = 0;

int expand_string(char** str, int len)
{
    int add = len;
    char* temp;
    while(add > 0)
    {
        temp = realloc(*str, (len + add) * sizeof(char));
        if(temp != NULL)
        {
            *str = temp;
            break;
        }
        add >>= 1;
    }
    return add;
}

int read_string(char** str, FILE* f)
{
    int fd = fileno(f);
    char* s = malloc(4 * sizeof(char));
    int buf = 4;
    int sz = 0;
    while(1)
    {
        int rd;
        if(sz == buf - 1)
        {
            if(expand_string(&s, buf) == 0)
            {
                free(s);
                return E_MALLOC;
            }
        }
        rd = read(fd, s + sz, sizeof(char));
        if(rd == -1)
        {
            if(rd == EAGAIN)
            {
                continue;
            }
            perror("read_string: ");
            *str = s;
            return E_CORPT;
        }
        if(rd == 0)
        {
            s[sz] = 0;
            *str = s;
            return E_OK;
        }
        if(rd != sizeof(char))
        {
            s[sz] = 0;
            *str = s;
            return E_CORPT;
        }
        if(s[sz] == '\n')
        {
            s[sz] = 0;
            *str = s;
            return E_OK;
        }
        if(s[sz] == EOF)
        {
            s[sz] = 0;
            *str = s;
            return E_EOF;
        }
        ++sz;
    }
    *str = s;
    return E_OK;
}

int main(int argc, const char** argv)
{
    int s = 0;
    char* buf;
    while(read_string(&buf, stdin) == E_OK)
    {
        printf("%s\n", buf);
        s += 1;
    }
    if(err)
    {
        return err;
    }
    printf("Number of lines = %d\n", s);
    return 0;
}