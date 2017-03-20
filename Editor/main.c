#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <errno.h>

#define E_LNRNG 1
#define E_MALLOC 2

typedef unsigned int ui32;

typedef struct lines
{
    wchar_t *s;
    int len;
    int buf;
    line *prev, *next;
} line;

int _line_init(line *l)
{
    l->s = malloc(sizeof(char));
    if(l->s == NULL)
    return E_MALLOC;
    s[0] = 0;
    l->len = 0;
    l->buf = 1;
    l->prev = l->next = NULL;

}

int _line_init(line* l, const char* s, int len)
{
    size_t buf = _log_2(len + 1);
    l->s = malloc(buf * sizeof(char));
    if(l->s == NULL)
        return E_MALLOC;
    strncpy(l->s, s, len);
    l->len = len;
    l->buf = buf;
    l->prev = l->next = NULL;
}

void _line_destroy(line *l)
{
    free(l->s);
    free(l);
}

size_t _log_2(size_t a)
{
    size_t ans = 0, t = 1;
    while(t < a)
    {
        ++ans;
        t <<= 1;
    }
    return ans;
}

size_t _expandarr(void**s, size_t len, size_t item)
{
    size_t add = (len > 0 ? len : 1);
    void *ans;
    while (add > 0 && (ans = realloc(*s, (len + add) * item)) == NULL)
    {
        add /= 2;
    }
    if (add > 0)
        *s = ans;
    return add + len;
}

int _refine_line(line *l)
{
    size_t segm = 1, buf = 2;
    size_t* arr = malloc(2 * sizeof(size_t));
    ui32 i;
    line *start, *end;
    arr[0] = 0;
    for(i = 0; i < l->len; ++i)
    {
        if(l->s[i] == '\n')
        {
            if(segm == buf - 1)
            {
                if((buf = _expandarr((void**)&arr, buf, sizeof(size_t))) - 1 == segm)
                    return E_MALLOC;
            }
            arr[segm] = i;
            ++segm;
        }
    }
    arr[segm] = l->len;
    start = malloc(sizeof(line));
    start->next = NULL;
    end = start;
    for(i = 0; i < segm; ++i)
    {
        int err;
        line* x = malloc(sizeof(line));
        size_t len = arr[i + 1] - arr[i];
        if((err == _line_init(x, s + arr[i], len)) == 0)
        {
            line* c = start;
            do
            {
                line* tmp = c->next;
                free(c);
                c = tmp;
            } while(c != NULL);
            return err;
        }
        x->prev = end;
        end->next = x;
        end = x;
    }
    l->prev->next = start->next;
    l->next->prev = end;
    _line_destroy(l);
    return 0;
}

int edit_string(line *l, size_t pos, wchar_t symb)
{
    if (pos >= l->len)
    {
        return E_LNRNG;
    }
    l->s[pos] = symb;
    return 0;
}

int insert_symbol(line *l, size_t pos, wchar_t symb)
{
    ssize_t i;
    if (l->len == l->buf - 1)
    {
        if ((l->buf = _expandarr((void**)&(l->s), l->buf, sizeof(wchar_t))) - 1 == l->len)
            return E_MALLOC;
    }
    ++len;
    for(i = l->len; i > pos; --i)
        l->s[pos] = l->s[pos - 1];
    l->s[pos] = symb;
    return 0;
}

int replace_substring(line* l, const char* sample, const char* repl)
{
    size_t slen = strlen(sample);
    size_t rlen = strlen(repl);
}


int main(int argc, const char *argv[])
{
    return 0;
}