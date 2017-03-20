#define _XOPEN_SOURCE 10

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <errno.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define E_STRRNG 1
#define E_MALLOC 2
#define E_LNRNG  3

typedef unsigned int ui32;

struct lines;

typedef struct lines line;

size_t pw2(size_t a)
{
    size_t t = 1;
    while(t < a)
    {
        t <<= 1;
    }
    return t;
}

struct lines
{
    wchar_t *s;
    int len;
    int buf;
    line *prev, *next;
};

int __line_init(line *l)
{
    l->s = malloc(sizeof(wchar_t));
    if(l->s == NULL)
        return E_MALLOC;
    l->s[0] = 0;
    l->len = 0;
    l->buf = 1;
    l->prev = l->next = NULL;
    return 0;
}

int __line_init_str(line *l, const wchar_t* s, int len)
{
    wprintf(L"%d\n", len);
    size_t buf = pw2(len + 1);
    l->s = malloc(buf * sizeof(wchar_t));
    if(l->s == NULL)
        return E_MALLOC;
    wcsncpy(l->s, s, len);
    l->s[len] = L'\0';
    l->len = len;
    l->buf = buf;
    l->prev = l->next = NULL;
    return 0;
}

void __line_destroy(line *l)
{
    free(l->s);
    free(l);
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

int _resize_line(line* l)
{
    size_t buf = pw2(l->len + 1);
    wchar_t* s = realloc(l->s, buf * sizeof(wchar_t));
    if(s == NULL)
        return E_MALLOC;
    l->buf = buf;
    l->s = s;
    return 0;
}

int _refine_line(line *l)
{
    size_t segm = 1, buf = 2;
    size_t* arr = malloc(2 * sizeof(size_t));
    size_t i;
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
            arr[segm] = i + 1;
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
        if((err = __line_init_str(x, l->s + arr[i], len)) != 0)
        {
            line* c = start;
            do
            {
                line* tmp = c->next;
                free(c);
                c = tmp;
            }
            while(c != NULL);
            free(arr);
            free(x);
            return err;
        }
        x->prev = end;
        end->next = x;
        end = x;
    }
    l->prev->next = start->next;
    start->next->prev = l->prev;
    l->next->prev = end;
    end->next = l->next;
    __line_destroy(l);
    free(arr);
    free(start);
    return 0;
}

void _delete_line(line *l)
{
    l->next->prev = l->prev;
    l->prev->next = l->next;
    __line_destroy(l);
}

int _merge_lines(line* left, line* right, size_t rstart)
{
    size_t buf = pw2(left->len + right->len + 1);
    wchar_t *s = realloc(left->s, buf * sizeof(wchar_t));
    if(s == NULL)
        return E_MALLOC;
    left->s = s;
    left->buf = buf;
    wcsncpy(left->s + right->len, right->s + rstart, right->len - rstart + 1);
    left->next = right->next;
    left->next->prev = left;
    __line_destroy(right);
    return 0;
}

line* _find_line(line* begin, line* end, size_t index)
{
    line* l = begin;
    int i = 0;
    while(l != end && i != index)
    {
        l = l->next;
        ++i;
    }
    if(l == end)
        return NULL;
    return l;
}

int edit_string(line *begin, line* end, size_t index, size_t pos, wchar_t symb)
{
    line* l = _find_line(begin, end, index);
    if(l == NULL)
        return E_LNRNG;
    if (pos >= l->len)
    {
        return E_STRRNG;
    }
    l->s[pos] = symb;
    return 0;
}

int insert_symbol(line *begin, line* end, size_t index, size_t pos, wchar_t symb)
{
    line* l = _find_line(begin, end, index);
    if(l == NULL)
        return E_LNRNG;
    ssize_t i;
    if(pos > l->len)
        pos = l->len;
    if(pos < 0)
        pos = 0;
    if (l->len == l->buf - 1)
    {
        if ((l->buf = _expandarr((void**)&(l->s), l->buf, sizeof(wchar_t))) - 1 == l->len)
            return E_MALLOC;
    }
    ++l->len;
    for(i = l->len; i > pos; --i)
        l->s[i] = l->s[i - 1];
    l->s[pos] = symb;
    return 0;
}

int replace_substring(line* l, const wchar_t* sample, const wchar_t* repl)
{
    /*TODO*/
}

int delete_range(line* begin, line* end, size_t first, size_t last)
{
    size_t i;
    line* cur = begin;
    for(i = 0; i < first - 1 && cur != end; ++i)
        cur = cur->next;
    for(; i < last - 1 && cur != end; ++i)
        _delete_line(cur->next);
    return 0;
}

int delete_braces(line* begin, line* end, size_t first, size_t last)
{
    size_t i = 0;
    line* cur = begin;

    size_t del_i;
    line* start;

    ui32 bal = 0;

    while(i != first && cur != end)
        cur = cur->next, ++i;
    if(cur == end)
        return E_LNRNG;
    while(i != last && cur != end)
    {
        size_t pos;
        size_t act_pos;
        size_t act_len = 0;
        for(pos = 0, act_pos = 0; pos < cur->len; ++pos)
        {
            if(cur->s[pos] == '{')
            {
                if(bal == 0)
                {
                    start = cur;
                    del_i = i;
                }
                ++bal;
            }
            if(bal == 0)
            {
                cur->s[act_pos++] = cur->s[pos];
                ++act_len;
            }
            else if(cur->s[pos] == '}')
            {
                --bal;
                if(bal == 0 && start != cur)
                {
                    size_t len = start->len;
                    size_t rlen = cur->len;
                    int err;
                    if((err = _merge_lines(start, cur, pos + 1)) != 0)
                        return err;
                    cur = start;
                    cur->len = len + rlen - pos - 1;
                    pos = act_pos = len;
                    --pos;
                    act_len = len;
                }
            }
        }
        cur->len = act_len;
        cur->s[cur->len] = 0;
        ++i;
        cur = cur->next;
    }
    if(bal != 0)
    {
        int err;
        delete_range(begin, end, del_i + 1, last);
        if((err = _resize_line(start)) != 0)
            return err;
    }
    return 0;
}

void print(line* begin, line* end)
{
    /*printf("start printing\n");*/
    line *l;
    for(l = begin->next; l != end; l = l->next)
    {
        wprintf(L"%ls\n", l->s);
    }
    wprintf(L"\n--------------------\n");
}

int main(int argc, const char *argv[])
{
    line* begin = malloc(sizeof(line));
    line* end = malloc(sizeof(line));
    line* c = malloc(sizeof(line));
    fflush(stdout);
    __line_init_str(c,L"a", 1);
    begin->next = c;
    c->prev = begin;
    c->next = end;
    end->prev = c;
    int i;
    for(i = 0; i < 10; ++i)
    {
        print(begin, end);
        insert_symbol(begin, end, 1, 0, L'a' + i + 1);
        edit_string(begin, end, 1, 0, L'a');
    }
    free(begin);
    free(end);

    return 0;
}