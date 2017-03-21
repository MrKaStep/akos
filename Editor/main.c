#define _XOPEN_SOURCE 10

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <errno.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define E_STRRNG 1
#define E_MALLOC 2
#define E_LNRNG  3
#define E_C_WRNG 4

#define C_SET_T    101
#define C_SET_N    102
#define C_PRINT_P  103
#define C_PRINT_R  104
#define C_SET_W    105
#define C_INSERT_A 106
#define C_EDIT_S   107
#define C_INSERT_S 108
#define C_REPL_S   109
#define C_DELETE_R 110
#define C_DELETE_B 111
#define C_EXIT     112
#define C_EXIT_F   113
#define C_READ     114
#define C_OPEN     115
#define C_WRITE    116
#define C_SET_NM   117
#define C_HELP     118

#define MAXWORD 11

typedef unsigned int ui32;

struct lines;

typedef struct lines line;

size_t pw2(size_t a)
{
    size_t t = 1;
    while (t < a)
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
    if (l->s == NULL)
        return E_MALLOC;
    l->s[0] = 0;
    l->len = 0;
    l->buf = 1;
    l->prev = l->next = NULL;
    return 0;
}

int __line_init_str(line *l, const wchar_t *s, int len)
{
    size_t buf = pw2(len + 1);
    l->s = malloc(buf * sizeof(wchar_t));
    if (l->s == NULL)
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

int _z_function(size_t **z, const wchar_t *str1, size_t len1, const wchar_t *str2, size_t len2)
{
    size_t l = 0, r = 0, i;
    size_t len = len1 + len2 + 1;
    size_t *Z;
    *z = malloc((len1 + len2 + 1) * sizeof(size_t));
    if (*z == NULL)
        return E_MALLOC;
    Z = *z;
    Z[0] = len;
    for (i = 0; i < len; ++i)
    {
        Z[i] = 0;
        if (r > i)
        {
            Z[i] = MIN(r - i, Z[i - l]);
        }
        while (i + Z[i] < len &&
                (i + Z[i] < len1 ? str1[i + Z[i]] : i + Z[i] > len1 ? str2[i + Z[i] - len1 - 1] : L'0') == (Z[i] < len1 ? str1[Z[i]] : Z[i] > len1 ? str2[Z[i] - len1 - 1] : L'0'))
            ++Z[i];
        if (i + Z[i] > r)
            l = i, r = i + Z[i];
    }
    return 0;
}

size_t _expandarr(void **s, size_t len, size_t item)
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

int _resize_line(line *l)
{
    size_t buf = pw2(l->len + 1);
    wchar_t *s = realloc(l->s, buf * sizeof(wchar_t));
    if (s == NULL)
        return E_MALLOC;
    l->buf = buf;
    l->s = s;
    return 0;
}

int _refine_line(line *l)
{
    size_t segm = 1, buf = 2;
    size_t *arr = malloc(2 * sizeof(size_t));
    size_t i;
    line *start, *end;
    arr[0] = 0;
    for (i = 0; i < l->len; ++i)
    {
        if (l->s[i] == '\n')
        {
            if (segm == buf - 1)
            {
                if ((buf = _expandarr((void **)&arr, buf, sizeof(size_t))) - 1 == segm)
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
    for (i = 0; i < segm; ++i)
    {
        int err;
        line *x = malloc(sizeof(line));
        size_t len = arr[i + 1] - arr[i];
        if ((err = __line_init_str(x, l->s + arr[i], len)) != 0)
        {
            line *c = start;
            do
            {
                line *tmp = c->next;
                free(c);
                c = tmp;
            }
            while (c != NULL);
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

int _merge_lines(line *left, line *right, size_t rstart)
{
    size_t buf = pw2(left->len + right->len + 1);
    wchar_t *s = realloc(left->s, buf * sizeof(wchar_t));
    if (s == NULL)
        return E_MALLOC;
    left->s = s;
    left->buf = buf;
    wcsncpy(left->s + right->len, right->s + rstart, right->len - rstart + 1);
    left->next = right->next;
    left->next->prev = left;
    __line_destroy(right);
    return 0;
}

line *_find_line(line *begin, line *end, size_t index)
{
    line *l = begin;
    int i = 0;
    while (l != end && i != index)
    {
        l = l->next;
        ++i;
    }
    if (l == end)
        return NULL;
    return l;
}

int insert_after(line *l, const wchar_t *s)
{
    line *t;
    size_t len;
    size_t buf;
    len = wcslen(s);
    buf = pw2(len);
    t = malloc(sizeof(line));
    if (t == NULL)
        return E_MALLOC;
    t->s = malloc(buf * sizeof(wchar_t));
    if (t->s == NULL)
    {
        free(t);
        return E_MALLOC;
    }
    wcsncpy(t->s, s, len);
    t->len = len;
    t->buf = buf;
    t->prev = l;
    t->next = l->next;
    t->prev->next = t;
    t->next->prev = t;
    _refine_line(t);
    return 0;
}

int edit_string(line *l, size_t pos, wchar_t symb)
{
    if (pos >= l->len)
    {
        return E_STRRNG;
    }
    l->s[pos] = symb;
    return 0;
}

int insert_symbol(line *l, size_t pos, wchar_t symb)
{
    ssize_t i;
    if (pos > l->len)
        pos = l->len;
    if (pos < 0)
        pos = 0;
    if (l->len == l->buf - 1)
    {
        if ((l->buf = _expandarr((void **)&(l->s), l->buf, sizeof(wchar_t))) - 1 == l->len)
            return E_MALLOC;
    }
    ++l->len;
    for (i = l->len; i > pos; --i)
        l->s[i] = l->s[i - 1];
    l->s[pos] = symb;
    return 0;
}

int replace_substring(line *l, const wchar_t *sample, const wchar_t *repl)
{
    size_t *z;
    size_t slen = wcslen(sample), rlen = wcslen(repl);
    size_t nlen, nbuf;
    wchar_t *buf;
    size_t subst = 0;
    size_t i, wi;
    _z_function(&z, sample, slen, l->s, l->len);
    for (i = 0; i < rlen;)
    {
        if (z[i + slen + 1] == slen)
        {
            ++subst;
            i += slen;
        }
        else
        {
            ++i;
        }
    }
    nlen = l->len - subst * slen + subst * rlen;
    nbuf = pw2(nlen);
    buf = malloc(nbuf * sizeof(wchar_t));
    if (buf == NULL)
        return E_MALLOC;
    i = 0;
    wi = 0;
    for (i = 0; i < l->len;)
    {
        if (z[i + slen + 1] == slen)
        {
            memcpy(buf + wi, repl, rlen * sizeof(wchar_t));
            wi += rlen;
            i += slen;
        }
        else
        {
            buf[wi] = l->s[i];
            ++i;
            ++wi;
        }
    }
    free(l->s);
    l->s = buf;
    l->buf = nbuf;
    l->len = nlen;
    return 0;
}

int delete_range(line *begin, line *end, size_t first, size_t last)
{
    size_t i;
    line *cur = begin;
    for (i = 0; i < first - 1 && cur != end; ++i)
        cur = cur->next;
    for (; i < last - 1 && cur != end; ++i)
        _delete_line(cur->next);
    return 0;
}

int delete_braces(line *begin, line *end, size_t first, size_t last)
{
    size_t i = 0;
    line *cur = begin;

    size_t del_i;
    line *start;

    ui32 bal = 0;

    while (i != first && cur != end)
        cur = cur->next, ++i;
    if (cur == end)
        return E_LNRNG;
    while (i != last && cur != end)
    {
        size_t pos;
        size_t act_pos;
        size_t act_len = 0;
        for (pos = 0, act_pos = 0; pos < cur->len; ++pos)
        {
            if (cur->s[pos] == '{')
            {
                if (bal == 0)
                {
                    start = cur;
                    del_i = i;
                }
                ++bal;
            }
            if (bal == 0)
            {
                cur->s[act_pos++] = cur->s[pos];
                ++act_len;
            }
            else if (cur->s[pos] == '}')
            {
                --bal;
                if (bal == 0 && start != cur)
                {
                    size_t len = start->len;
                    size_t rlen = cur->len;
                    int err;
                    if ((err = _merge_lines(start, cur, pos + 1)) != 0)
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
    if (bal != 0)
    {
        int err;
        delete_range(begin, end, del_i + 1, last);
        if ((err = _resize_line(start)) != 0)
            return err;
    }
    return 0;
}

void print(line *begin, line *end)
{
    /*printf("start printing\n");*/
    line *l;
    for (l = begin->next; l != end; l = l->next)
    {
        wprintf(L"%ls\n", l->s);
    }
    wprintf(L"\n--------------------\n");
}

int get_word(wchar_t* s, char* eoln)
{
    size_t n = 0;
    wchar_t c;
    *eoln = 0;
    while(iswspace(c = fgetwc(stdin)) && c != L'\n');
    if(c == L'\n')
    {
        *eoln = 1;
        return E_C_WRNG;
    }
    s[n++] = c;
    while(n < MAXWORD && iswalpha(s[n++] = fgetwc(stdin)));
    if(s[n - 1] == L'\n')
        *eoln = 1;
    if(n == MAXWORD)
        return E_C_WRNG;
    --n;
    s[n] = L'\0';
    return 0;
}

int get_command()
{
    wchar_t *w1, *w2;
    int err;
    int ret = 0;
    char eoln;
    fputws(L"editor: ", stdout);
    w1 = malloc(MAXWORD * sizeof(wchar_t));
    if(w1 == NULL)
        return E_MALLOC;
    w2 = malloc(MAXWORD * sizeof(wchar_t));
    if(w2 == NULL)
        return E_MALLOC;
    if((err = get_word(w1, &eoln)) != 0)
    {
        free(w1);
        free(w2);
        return err;
    }
    if(wcscmp(w1, L"exit") == 0)
        if(!eoln && get_word(w2, &eoln) == 0 && wcscmp(w1, L"force") == 0)
            ret = C_EXIT_F;
        else
            ret = C_EXIT;
    else if(wcscmp(w1, L"read") == 0)
        ret = C_READ;
    else if(wcscmp(w1, L"open") == 0)
        ret = C_OPEN;
    else if(wcscmp(w1, L"write") == 0)
        ret = C_WRITE;
    else if(wcscmp(w1, L"help") == 0)
        ret = C_HELP;
    else if(eoln)
        return E_C_WRNG;
    if(ret != 0)
    {
        free(w1);
        free(w2);
        return ret;
    }
    if((err = get_word(w2, &eoln)) != 0)
    {
        free(w1);
        free(w2);
        return err;
    }
    if(wcscmp(w1, L"set") == 0)
        if(wcscmp(w2, L"tabwidth") == 0)
            ret = C_SET_T;
        else if(wcscmp(w2, L"numbers") == 0)
            ret = C_SET_N;
        else if(wcscmp(w2, L"wrap") == 0)
            ret = C_SET_W;
        else if(wcscmp(w2, L"name") == 0)
            ret = C_SET_NM;
        else
            ret = E_C_WRNG;
    else if(wcscmp(w1, L"print") == 0)
        if(wcscmp(w2, L"pages") == 0)
            ret = C_PRINT_P;
        else if(wcscmp(w2, L"range") == 0)
            ret = C_PRINT_R;
        else
            ret = E_C_WRNG;
    else if(wcscmp(w1, L"insert") == 0)
        if(wcscmp(w2, L"after") == 0)
            ret = C_INSERT_A;
        else if(wcscmp(w2, L"symbol") == 0)
            ret = C_INSERT_S;
        else
            ret = E_C_WRNG;
    else if(wcscmp(w1, L"edit") == 0)
        if(wcscmp(w2, L"string") == 0)
            ret = C_EDIT_S;
        else
            ret = E_C_WRNG;
    else if(wcscmp(w1, L"replace") == 0)
        if(wcscmp(w2, L"substring") == 0)
            ret = C_REPL_S;
        else
            ret = E_C_WRNG;
    else if(wcscmp(w1, L"delete") == 0)
        if(wcscmp(w2, L"range") == 0)
            ret = C_DELETE_R;
        else if(wcscmp(w2, L"braces") == 0)
            ret = C_DELETE_B;
        else
            ret = E_C_WRNG;
    else
        ret = E_C_WRNG;
    free(w1);
    free(w2);
    return ret;
}



int main(int argc, const char *argv[])
{
    int a, b;
    int c = wscanf(L"%d%d\n", &a, &b);
    int t = get_command();
    if(t == C_EXIT)
        fputws(L"Bye!\n", stdout);
    if(t == C_DELETE_R)
        fputws(L"del_r\n", stdout);
    return 0;
}