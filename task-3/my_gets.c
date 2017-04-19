#define _XOPEN_SOURCE 1000

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define E_OK     0
#define E_MALLOC 1
#define E_EOF    2

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

int read_string(FILE* f, char** str)
{
    char* s = malloc(4 * sizeof(char));
    int buf = 4;
    int sz = 0;
    if(s == NULL)
        return E_MALLOC;
    while(1)
    {
        int c;
        if(sz == buf - 1)
        {
            if(expand_string(&s, buf) == 0)
            {
                free(s);
                return E_MALLOC;
            }
        }
        c = fgetc(f);
        if(c == EOF)
        {
            s[sz] = '\0';
            *str = s;
            return E_EOF;
        }
        s[sz] = c;
        if(s[sz] == '\n')
        {
            s[sz] = '\0';
            *str = s;
            return E_OK;
        }
        ++sz;
    }
}

int main(int argc, const char* argv[])
{
    int s = 0;
    char* buf;
    FILE* f;
    if(argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;
    while(read_string(f, &buf) == E_OK)
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