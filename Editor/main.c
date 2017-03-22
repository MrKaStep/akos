#define _XOPEN_SOURCE 10

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>
#include <errno.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define E_STRRNG 1
#define E_MALLOC 2
#define E_LNRNG  3
#define E_C_WRNG 4
#define E_NOARG  5
#define E_BADSYM 6
#define E_NARROW 7
#define E_IOFAIL 8

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

#define M_WRAP 1
#define M_NUM  2

#define B_ANY   0
#define B_LEFT  1
#define B_RIGHT 2
#define B_SPACE 3
#define B_Q     4

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

#define MAXWORD 16

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

size_t _expand_array(void **s, size_t len, size_t item)
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
                if ((buf = _expand_array((void **)&arr, buf, sizeof(size_t))) - 1 == segm)
                    return E_MALLOC;
            }
            arr[segm] = i + 1;
            ++segm;
        }
    }
    arr[segm] = l->len + 1;
    start = malloc(sizeof(line));
    start->next = NULL;
    end = start;
    for (i = 0; i < segm; ++i)
    {
        int err;
        line *x = malloc(sizeof(line));
        size_t len = arr[i + 1] - arr[i] - 1;
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

line* _find_line(line *begin, line *end, size_t index)
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

int _print_line(line* l, int *complete, char mode, size_t width, FILE* stream,...)
{
    wchar_t space[10], num[10];
    va_list args;
    va_start(args, stream);
    if(mode & M_NUM)
    {
        ui32 nlen;
        size_t lnum;
        wchar_t form[10];
        nlen = va_arg(args, ui32);
        lnum = va_arg(args, size_t);
        if(width <= nlen + 1)
            return E_IOFAIL;
        width -= nlen + 1;
        swprintf(form, 10, L"%%%dd ", nlen);
        swprintf(num, 10, form, lnum);
        swprintf(space, 10, L"         ");
        space[nlen + 1] = L'\0';
    }
    else if(mode & M_WRAP)
    {
        num[0] = L'-';
        num[1] = L' ';
        num[2] = L'\0';
        space[0] = L' ';
        space[1] = L' ';
        space[2] = L'\0';
        width -= 2;
    }
    else
    {
        num[0] = L'\0';
        space[0] = L'\0';
    }
    if(mode & M_WRAP)
    {
        size_t* height;
        size_t cur = 0;
        height = va_arg(args, size_t*);
        while(*height > 0 && cur < l->len)
        {
            size_t i;
            fwprintf(stream, L"%s%ls%s", GREEN, cur == 0 ? num : space, RESET);
            for(i = cur; i < MIN(l->len, cur + width); ++i)
                if(fputwc(l->s[i], stream) == WEOF)
                    return E_IOFAIL;
            cur += width;
            if(fputwc(L'\n', stream) == WEOF)
                return E_IOFAIL;
            --*height;
        }
        /*wprintf(L"---\n%llu %llu\n---\n", cur, l->len);*/
        *complete = (cur >= l->len);
    }
    else
    {
        size_t offset, i;
        offset = va_arg(args, size_t);
        fwprintf(stream, L"%s%ls%s", GREEN, num, RESET);
        for(i = offset; i < MIN(l->len, offset + width); ++i)
            if(fputwc(l->s[i], stream) == WEOF)
                return E_IOFAIL;
        if(fputwc(L'\n', stream) == WEOF)
            return E_IOFAIL;
        *complete = 1;
    }
    va_end(args);
    return 0;
}


wchar_t spec_symb(wchar_t symb)
{
    switch(symb)
    {
    case 'n':
        return '\n';
    case 't':
        return '\t';
    case 'r':
        return '\r';
    case '\\':
        return '\\';
    case '\"':
        return '\"';
    default:
        return symb;
    }
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
        if ((l->buf = _expand_array((void **)&(l->s), l->buf, sizeof(wchar_t))) - 1 == l->len)
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

void clr()
{
    wchar_t c;
    while((c = fgetwc(stdin)) != L'\n');
}

int get_word(wchar_t* s, char* eoln)
{
    size_t n = 0;
    wchar_t c;
    *eoln = 0;
    while(iswspace(c = fgetwc(stdin)) && c != L'\n');
    if(!iswalpha(c))
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
    clr();
    return 0;
}

int get_quoted_str(wchar_t** s)
{
    wchar_t c;
    size_t buf, len;
    ;
    while(iswspace(c = fgetwc(stdin)) && c != L'\n');
    if(c != L'\"')
        return E_NOARG;
    *s = malloc(16 * sizeof(wchar_t));
    if(*s == NULL)
        return E_MALLOC;
    buf = 16;
    len = 0;
    while((c = fgetwc(stdin)) != L'\"' && c != L'\n')
    {
        if(len == buf - 1)
        {
            buf = _expand_array((void**)s, buf, sizeof(wchar_t));
            if(len == buf - 1)
            {
                free((*s));
                return E_MALLOC;
            }
        }
        if(c == L'\\')
        {
            c = spec_symb(fgetwc(stdin));
            if(c == WEOF)
                continue;
        }
        (*s)[len++] = c;
    }
    if(c == L'\n')
    {
        free((*s));
        return E_NOARG;
    }
    (*s)[len] = L'\0';
    if(c != L'\n')
        clr();
    return 0;
}

int get_triple_quoted_str(wchar_t** s)
{
    wchar_t c;
    size_t buf, len;
    wchar_t lst[3] = {L'\0', L'\0', L'\0'};
    size_t cur = 0;
    char start = 0;
    while(iswspace(c = fgetwc(stdin)) && c != L'\n');
    if(c != L'\"')
        return E_NOARG;
    c = fgetwc(stdin);
    if(c != L'\"')
    {
        ungetwc(c, stdin);
        ungetwc(L'\"', stdin);
        return get_quoted_str(s);
    }
    c = fgetwc(stdin);
    if(c != L'\"')
    {
        *s = malloc(sizeof(wchar_t));
        (*s)[0] = L'\0';
        clr();
        return 0;
    }
    *s = malloc(16 * sizeof(wchar_t));
    if(*s == NULL)
        return E_MALLOC;
    buf = 16;
    len = 0;
    while(lst[0] != L'\"' || lst[1] != L'\"' || lst[2] != L'\"')
    {
        c = fgetwc(stdin);
        if(len == 0 && !start && c == L'\n')
        {
            start = 1;
            continue;
        }
        if(len == buf - 1)
        {
            buf = _expand_array((void**)s, buf, sizeof(wchar_t));
            if(len == buf - 1)
            {
                free((*s));
                return E_MALLOC;
            }
        }
        if(c == L'\\')
        {
            c = spec_symb(fgetwc(stdin));
            if(c == WEOF)
                continue;
        }
        (*s)[len++] = c;
        lst[(++cur + 1) % 3] = c;
    }
    len = len - 3;
    (*s)[len] = L'\0';
    clr();
    return 0;
}

int get_int(int* a)
{
    wchar_t c;
    while(iswspace(c = fgetwc(stdin)) && c != L'\n');
    if(!iswdigit(c))
        return E_NOARG;
    ungetwc(c, stdin);
    wscanf(L"%d", a);
    return 0;
}

int get_button()
{
    switch(fgetwc(stdin))
    {
    case L'q':
        return B_Q;
    case L' ':
        return B_SPACE;
    case L'\033':
        fgetwc(stdin);
        switch(fgetwc(stdin))
        {
        case L'C':
            return B_RIGHT;
        case L'D':
            return B_LEFT;
        default:
            return B_ANY;
        }
    default:
        return B_ANY;
    }
}

int print_pages(line* begin, line* end, size_t first, size_t last, FILE* stream, char num, char wrap)
{
    size_t cur;
    line* l;
    int err;
    int complete;
    l = _find_line(begin, end, first);
    if(l == NULL)
        return E_LNRNG;
    if(!isatty(fileno(stream)))
    {
        if(num)
        {
            ui32 nlen;
            size_t tmp = last;
            nlen = 0;
            while(tmp > 0)
                tmp /= 10, ++nlen;
            for(cur = first; cur < last; ++cur, l = l->next)
                if((err = _print_line(l, &complete, M_NUM, -1, stream, nlen, cur, 0)) != 0)
                    return err;
        }
        else
            for(cur = first; cur < last; ++cur, l = l->next)
                if((err = _print_line(l, &complete, 0, -1, stream, 0)) != 0)
                    return err;
        return 0;
    }
    else
    {
        struct termios old_term, new_term;
        struct winsize sz;
        size_t height, width, offset = 0, cur_height, old_cur;
        tcgetattr(0, &old_term);
        memcpy(&new_term, &old_term, sizeof(struct termios));
        new_term.c_lflag &= ~ECHO;
        new_term.c_lflag &= ~ICANON;
        new_term.c_cc[VMIN] = 1;
        tcsetattr(0, TCSANOW, &new_term);

        ioctl(1, TIOCGWINSZ, &sz);
        height = sz.ws_row - 1;
        width = sz.ws_col;
        cur = old_cur = first;
        while(1)
        {
            complete = 0;
            fwprintf(stream, L"\n");
            cur = old_cur;
            l = _find_line(begin, end, cur);
            if(l == NULL)
            {
                tcsetattr(0, TCSANOW, &old_term);
                return E_LNRNG;
            }
            cur_height = height;
            if(num)
            {
                ui32 nlen;
                size_t tmp = last;
                nlen = 0;
                while(tmp > 0)
                    tmp /= 10, ++nlen;
                while(cur_height > 0)
                {
                    if(cur == last)
                    {
                        while(cur_height-- > 0)
                            fwprintf(stream, L"%s~%s\n", BLUE, RESET);
                        break;
                    }
                    else if(wrap)
                    {
                        if((err = _print_line(l, &complete, M_NUM | M_WRAP, width, stream, nlen, cur, &cur_height)) != 0)
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                    }
                    else
                    {
                        if((err = _print_line(l, &complete, M_NUM, width, stream, nlen, cur, offset)) != 0)
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        --cur_height;
                    }
                    ++cur;
                    l = l->next;
                }
            }
            else
            {
                while(cur_height > 0)
                {
                    if(cur == last)
                    {
                        while(cur_height-- > 0)
                            fwprintf(stream, L"%s~%s\n", BLUE, RESET);
                        break;
                    }
                    else if(wrap)
                    {
                        if((err = _print_line(l, &complete, M_WRAP, width, stream, &cur_height)) != 0)
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                    }
                    else
                    {
                        if((err = _print_line(l, &complete, 0, width, stream, offset)) != 0)
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        --cur_height;
                    }
                    ++cur;
                    l = l->next;
                }
            }
            fwprintf(stream, L"\033[30;47m Выход: q, След. страница: SPACE");
            if(!wrap)
                fwprintf(stream, L", Навигация: LEFT, RIGHT");
            fwprintf(stream, L" %s", RESET);
            while(1)
            {
                char ready = 0;
                switch(get_button())
                {
                case B_LEFT:
                    cur = old_cur;
                    if(offset != 0)
                        --offset;
                    ready = 1;
                    break;
                case B_RIGHT:
                    cur = old_cur;
                    ++offset;
                    ready = 1;
                    break;
                case B_SPACE:
                    if(cur != last || !complete)
                    {
                        old_cur = cur - 1;
                        offset = 0;
                        ready = 1;
                        break;
                    }
                case B_Q:
                    tcsetattr(0, TCSANOW, &old_term);
                    fwprintf(stream, L"\n");
                    return 0;
                default:
                    continue;
                }
                if(ready)
                    break;
            }
        }
    }
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
    line *begin, *end;
    int i;
    setlocale(LC_ALL, "ru_RU.utf8");
    begin = malloc(sizeof(line));
    end = malloc(sizeof(line));
    __line_init(begin);
    __line_init(end);
    begin->next = end;
    end->prev = begin;
    for(i = 0; i < 100; ++i)
        insert_after(begin, L"aaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbcccccccccccccccccccccccccdddddddddddddddddddddddddddddddddddeeeeeeeeeeeeeeeeeeefffffffffffffffff");
    print_pages(begin, end, 1, 101, stdout, 1, 0);
    return 0;
}
