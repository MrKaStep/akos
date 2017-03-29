#define _XOPEN_SOURCE 1000

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

#include <linux/limits.h>



#define E_MALLOC 1
#define E_IOFAIL 2
#define E_STRRNG 3
#define E_LNRNG  4
#define E_C_WRNG 5
#define E_NOARG  6
#define E_NARROW 7
#define E_NOFILE 8
#define E_COMM   9
#define E_EMPTY  10
#define E_EOF    11
#define E_NOTSAV 12

#define C_SET_T    101  /*set tabwidth*/
#define C_SET_N    102  /*set numbers*/
#define C_PRINT_P  103  /*print pages*/
#define C_PRINT_R  104  /*print range*/
#define C_SET_W    105  /*set wrap*/
#define C_INSERT_A 106  /*insert after*/
#define C_EDIT_S   107  /*edit string*/
#define C_INSERT_S 108  /*insert symbol*/
#define C_REPL_S   109  /*replace substring*/
#define C_DELETE_R 110  /*delete range*/
#define C_DELETE_B 111  /*delete braces*/
#define C_EXIT     112  /*exit*/
#define C_EXIT_F   113  /*exit force*/
#define C_READ     114  /*read*/
#define C_OPEN     115  /*open*/
#define C_WRITE    116  /*write*/
#define C_SET_NM   117  /*set name*/
#define C_HELP     118  /*help*/

#define M_WRAP 1
#define M_NUM  2
#define M_TAB  4

#define B_ANY   0
#define B_LEFT  1
#define B_RIGHT 2
#define B_SPACE 3
#define B_Q     4

#define M_INSERT 1
#define M_EDIT   2

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

#define MAXWORD 16

#define FUCK wprintf(L"FUCK\n");

typedef unsigned int ui32;

struct lines;

typedef struct lines line;

size_t tab_width = 8;


size_t min(size_t a, size_t b)
{
    if(a > b)
        return b;
    return a;
}


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
    size_t len;
    size_t buf;
    line *prev, *next;
};

int __line_init(line *l)
{
    l->s = calloc(1, sizeof(wchar_t));
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
    l->s = calloc(1, buf * sizeof(wchar_t));
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

/*wchar_t _wcscat_at(const wchar_t *str1, size_t len1, const wchar_t *str2, size_t len2, size_t pos)
{
    if(pos < len1)
        return str1[pos];
    if(pos == len1)
        return L'\0';
    return str2[pos - len1 - 1];
}*/

int _z_function(size_t **z, const wchar_t *str1, size_t len1, const wchar_t *str2, size_t len2)
{
    size_t l = 0, r = 0, i;
    size_t len = len1 + len2 + 1;
    size_t *Z;
    *z = calloc(1, (len1 + len2 + 1) * sizeof(size_t));
    if (*z == NULL)
        return E_MALLOC;
    Z = *z;
    Z[0] = len;
    for (i = 0; i < len; ++i)
    {
        Z[i] = 0;
        if (r > i)
        {
            Z[i] = min(r - i, Z[i - l]);
        }
        while (i + Z[i] < len &&
                (i + Z[i] < len1 ? str1[i + Z[i]] : i + Z[i] > len1 ? str2[i + Z[i] - len1 - 1] : L'\0') == (Z[i] < len1 ? str1[Z[i]] : Z[i] > len1 ? str2[Z[i] - len1 - 1] : L'\0'))
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

int _refine_line(line *l, size_t* total)
{
    size_t segm = 1, buf = 2;
    size_t *arr = calloc(1, 2 * sizeof(size_t));
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
    *total = segm;
    arr[segm] = l->len + 1;
    start = calloc(1, sizeof(line));
    start->next = NULL;
    end = start;
    for (i = 0; i < segm; ++i)
    {
        int err;
        line *x = calloc(1, sizeof(line));
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

int _merge_lines(line *left, line *right, size_t rstart, size_t *add)
{
    size_t buf = pw2(left->len + right->len + 1);
    wchar_t *s = realloc(left->s, buf * sizeof(wchar_t));
    *add = 1;
    while(left->next != right)
    {
        ++*add;
        _delete_line(left->next);
    }
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
    size_t i = 0;
    while ((l != end) && (i != index))
    {
        l = (l->next);
        ++i;
    }
    return l;
}

int _print_line(line* l, char mode, size_t width, size_t offset, FILE* stream,...)
{
    wchar_t space[10], num[10];
    size_t add = 0;
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
        size_t cur = offset;
        size_t* complete = va_arg(args, size_t*);
        height = va_arg(args, size_t*);
        if(offset >= l->len)
        {
            fwprintf(stream, L"%s%ls%s\n", GREEN, num, RESET);
            --*height;
        }
        while(*height > 0 && cur < l->len)
        {
            size_t i;
            fwprintf(stream, L"%s%ls%s", GREEN, cur == offset ? num : space, RESET);
            for(i = cur; i + add < min(l->len + add, cur + width); ++i)
                if((mode & M_TAB) && l->s[i] == '\t')
                {
                    size_t j;
                    for(j = 0; j < tab_width && i + add < min(l->len + add, cur + width); ++j, ++add)
                        if(fputwc(L' ', stream) == WEOF)
                            return E_IOFAIL;
                }
                else if(fputwc(l->s[i], stream) == WEOF)
                    return E_IOFAIL;

            cur += width;
            if(fputwc(L'\n', stream) == WEOF)
                return E_IOFAIL;
            --*height;
        }
        *complete = (cur >= l->len ? 0 : cur);
    }
    else
    {
        size_t i;
        fwprintf(stream, L"%s%ls%s", GREEN, num, RESET);
        for(i = offset; i + add < min(l->len + add, offset + width); ++i)
            if((mode & M_TAB) && l->s[i] == '\t')
            {
                size_t j;
                for(j = 0; j < tab_width && i + add < min(l->len + add, offset + width); ++j, ++add)
                    if(fputwc(L' ', stream) == WEOF)
                        return E_IOFAIL;
            }
            else if(fputwc(l->s[i], stream) == WEOF)
                return E_IOFAIL;
        if(fputwc(L'\n', stream) == WEOF)
            return E_IOFAIL;
    }
    va_end(args);
    return 0;
}

int _replace_substring(line *l, const wchar_t *sample, const wchar_t *repl, size_t* total)
{
    size_t *z;
    size_t slen = wcslen(sample), rlen = wcslen(repl);
    size_t nlen, nbuf;
    wchar_t *buf;
    size_t subst = 0;
    size_t i, wi;
    int err;
    if(slen == 0)
        return E_NOARG;
    if(wcscmp(sample, L"^") == 0 || wcscmp(sample, L"$") == 0)
    {
        nbuf = pw2(l->len + rlen);
        nlen = l->len + rlen;
        buf = calloc(1, pw2(l->len + rlen + 1) * sizeof(wchar_t));
        if(buf == NULL)
            return E_MALLOC;
        if(sample[0] == L'^')
        {
            memcpy(buf, repl, rlen * sizeof(wchar_t));
            memcpy(buf + rlen, l->s, l->len * sizeof(wchar_t));
        }
        else
        {
            memcpy(buf, l->s, l->len * sizeof(wchar_t));
            memcpy(buf + l->len, repl, rlen * sizeof(wchar_t));
        }
        buf[l->len + rlen] = L'\0';
        free(l->s);
        l->s = buf;
        l->buf = nbuf;
        l->len = nlen;
        if((err = _refine_line(l, total)))
            return err;
        return 0;
    }


    _z_function(&z, sample, slen, l->s, l->len);

    for (i = 0; i < l->len;)
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
    nbuf = pw2(nlen + 1);
    buf = calloc(1, nbuf * sizeof(wchar_t));
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
    free(z);
    free(l->s);
    l->s = buf;
    l->buf = nbuf;
    l->len = nlen;
    if((err = _refine_line(l, total)))
        return err;
    return 0;
}


int _get_sentence(wchar_t** _buf)
{
    char quotes_flag = 0;
    char quotes = 0;
    wchar_t *s;
    wint_t c;
    size_t len = 0, buf = 16;
    s = calloc(1, 16 * sizeof(wchar_t));
    if(s == NULL)
        return E_MALLOC;
    while((c = fgetwc(stdin)) != WEOF && (c != L'\n' || quotes_flag))
    {
        if(len + 1 == buf)
        {
            buf = _expand_array((void**)&s, buf, sizeof(wchar_t));
            if(len + 1 == buf)
            {
                free(s);
                *_buf = NULL;
                return E_MALLOC;
            }
        }
        s[len++] = c;
        if(c == L'\"')
        {
            if(++quotes == 3)
                quotes_flag ^= 1;
            quotes %= 3;
        }
        else
            quotes = 0;
    }
    if(c == WEOF)
    {
        free(s);
        *_buf = NULL;
        return E_EOF;
    }
    s[len] = L'\0';
    *_buf = s;
    while(iswspace(*s))
        ++s;
    if(*s == '#')
    {
        free(*_buf);
        *_buf = NULL;
        return E_COMM;
    }
    else if(s == *_buf + len)
    {
        free(*_buf);
        *_buf = NULL;
        return E_EMPTY;
    }
    return 0;
}


wchar_t spec_symb(wchar_t symb)
{
    switch(symb)
    {
    case L'n':
        return L'\n';
    case L't':
        return L'\t';
    case L'r':
        return L'\r';
    case L'\\':
        return L'\\';
    case L'\"':
        return L'\"';
    default:
        return symb;
    }
}

int insert_after(line *l, const wchar_t *s, size_t *total)
{
    line *t;
    size_t len;
    size_t buf;
    len = wcslen(s);
    buf = pw2(len);
    t = calloc(1, sizeof(line));
    if (t == NULL)
        return E_MALLOC;
    t->s = calloc(1, buf * sizeof(wchar_t));
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
    _refine_line(t, total);
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
    size_t i;
    if (pos > l->len)
        pos = l->len;
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

int delete_range(line *begin, line *end, size_t first, size_t last, size_t *total)
{
    size_t i;
    line *cur = begin;
    *total = 0;
    for (i = 0; i < first - 1 && cur != end; ++i)
        cur = cur->next;
    for (; i < last - 1 && cur->next != end; ++i)
        _delete_line(cur->next), ++*total;
    return 0;
}

int delete_braces(line *begin, line *end, size_t first, size_t last, size_t *total)
{
    size_t i = 0;
    line *cur = begin;

    size_t del_i;
    line *start;

    ui32 bal = 0;

    *total = 0;


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
                    size_t add;
                    int err;
                    if ((err = _merge_lines(start, cur, pos + 1, &add)) != 0)
                        return err;
                    cur = start;
                    cur->len = len + rlen - pos - 1;
                    pos = act_pos = len;
                    --pos;
                    act_len = len;
                    *total += add;
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
        size_t tmp;
        if((err = delete_range(begin, end, del_i + 1, last, &tmp)))
            return err;
        *total += tmp;
        if ((err = _resize_line(start)))
            return err;
    }
    return 0;
}

int replace_substring(line* begin, line* end, size_t first, size_t last,
                      const wchar_t* sample, const wchar_t* repl, size_t* total)
{
    int err;
    line *l, *nxt;
    size_t cur = first;
    l = _find_line(begin, end, first);
    *total = 0;
    if(l == NULL)
        return E_LNRNG;
    while(l != end && cur != last)
    {
        size_t tmp;
        nxt = l->next;
        if((err = _replace_substring(l, sample, repl, &tmp)))
            return err;
        *total += tmp - 1;
        l = nxt;
    }
    return 0;
}

int get_word(wchar_t **_buf, wchar_t* s)
{
    size_t n = 0;
    while(iswspace(**_buf) && **_buf != L'\n')
        ++*_buf;
    if(!iswalpha(**_buf))
        return E_C_WRNG;
    s[n++] = **_buf;
    ++*_buf;
    while(n < MAXWORD && iswalpha(s[n++] = **_buf))
        ++*_buf;
    if(n == MAXWORD)
        return E_C_WRNG;
    --n;
    s[n] = L'\0';
    return 0;
}


int get_quoted_string(wchar_t** _buf, wchar_t** s)
{
    wchar_t *first, *last, *i;
    size_t spec = 0, len = 0;
    char spec_flag = 1;
    while(iswspace(**_buf) && **_buf != L'\n')
        ++*_buf;
    if(**_buf != L'\"')
        return E_NOARG;
    ++*_buf;
    first = *_buf;
    while(**_buf != L'\"' && **_buf != L'\n' && **_buf != L'\0')
    {
        if(spec_flag && **_buf == L'\\')
        {
            ++spec;
            spec_flag = 0;
        }
        else
            spec_flag = 1;
        ++*_buf;
    }
    if(**_buf == L'\n' || **_buf == L'\0')
        return E_NOARG;
    last = (*_buf)++;
    *s = calloc(1, pw2(last - first - spec + 1) * sizeof(wchar_t));
    if(s == NULL)
        return E_MALLOC;
    for(i = first; i != last; ++i)
    {
        wchar_t c;
        if(*i == L'\\')
        {
            ++i;
            if(*i == L'\n' || *i == L'\0')
                continue;
            c = spec_symb(*i);
        }
        else
            c = *i;
        (*s)[len++] = c;
    }
    (*s)[len] = L'\0';
    return 0;
}

int get_triple_quoted_string(wchar_t** _buf, wchar_t** s)
{
    wchar_t *first, *last, *i;
    size_t len = 0, quotes = 0, spec = 0;
    char spec_flag = 1;
    while(iswspace(**_buf) && **_buf != L'\n')
        ++*_buf;
    if(**_buf != L'\"')
        return E_NOARG;
    ++*_buf;
    if(**_buf != L'\"')
    {
        *_buf -= 2;
        return get_quoted_string(_buf, s);
    }
    ++*_buf;
    if(**_buf != L'\"')
    {
        *s = calloc(1, sizeof(wchar_t));
        (*s)[0] = L'\0';
        return 0;
    }
    ++*_buf;
    first = *_buf;
    while(**_buf != L'\0' && quotes != 3)
    {
        if(spec_flag && **_buf == L'\\')
        {
            ++spec;
            spec_flag = 0;
        }
        else
            spec_flag = 1;
        if(**_buf == L'\"')
            ++quotes;
        else
            quotes = 0;
        ++*_buf;
    }
    if(quotes != 3)
        return E_NOARG;
    last = *_buf - 3;
    if(*first == L'\n')
        ++first;
    if(last != first && *(last - 1) == L'\n' && *(last - 2) != L'\\')
        --last;
    *s = calloc(1, (last - first - spec + 1) * sizeof(wchar_t));
    for(i = first; i != last; ++i)
    {
        wchar_t c;
        c = *i;
        if(c == L'\\')
        {
            ++i;
            c = spec_symb(*i);
        }
        (*s)[len++] = c;
    }
    (*s)[len] = L'\0';
    return 0;
}

int get_int(wchar_t **_buf, size_t* a)
{
    while(iswspace(**_buf) && **_buf != L'\0')
        ++*_buf;
    if(!iswdigit(**_buf) || **_buf == L'\0')
        return E_NOARG;
    *a = 0;
    while(iswdigit(**_buf))
    {
        *a *= 10;
        *a += (**_buf - L'0');
        ++*_buf;
    }
    return 0;
}

int get_symb(wchar_t **buf, wchar_t* c, char spec)
{
    while(iswspace(**buf))
        ++*buf;
    if(**buf == L'\0')
        return E_NOARG;
    if(spec && **buf == '\\')
    {
        ++*buf;
        if(get_symb(buf, c, 0))
            return E_NOARG;
        *c = spec_symb(*c);
        return 0;
    }
    *c = **buf;
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
                if((err = _print_line(l, M_NUM, -1, 0, stream, nlen, cur)))
                    return err;
        }
        else
            for(cur = first; cur < last; ++cur, l = l->next)
                if((err = _print_line(l, 0, -1, 0, stream)))
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
        if(!isatty(0))
            ++height;
        width = sz.ws_col;
        cur = old_cur = first;

        while(1)
        {
            cur_height = height;
            fwprintf(stream, L"\n");
            cur = old_cur;
            l = _find_line(begin, end, cur);
            if(l == NULL)
            {
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
                    if(cur >= last)
                    {
                        while(cur_height-- > 0)
                            fwprintf(stream, L"%s~%s\n", BLUE, RESET);
                        break;
                    }
                    if(wrap)
                    {
                        if((err = _print_line(l, M_NUM | M_WRAP | M_TAB, width, offset, stream, nlen, cur, &offset, &cur_height)))
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        if(offset == 0)
                        {
                            ++cur;
                            l = l->next;
                        }
                    }
                    else
                    {
                        if((err = _print_line(l, M_NUM | M_TAB, width, offset, stream, nlen, cur)))
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        ++cur;
                        l = l->next;
                        --cur_height;
                    }
                }
            }
            else
            {
                while(cur_height > 0)
                {
                    if(cur >= last)
                    {
                        while(cur_height-- > 0)
                            fwprintf(stream, L"%s~%s\n", BLUE, RESET);
                        break;
                    }
                    if(wrap)
                    {
                        if((err = _print_line(l, M_WRAP | M_TAB, width, offset, stream, &offset, &cur_height)))
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        if(offset == 0)
                        {
                            ++cur;
                            l = l->next;
                        }
                    }
                    else
                    {
                        if((err = _print_line(l, M_TAB, width, offset, stream)))
                        {
                            tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        ++cur;
                        l = l->next;
                        --cur_height;
                    }
                }
            }
            if(isatty(fileno(stdin)))
            {
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
                        if(wrap)
                            continue;
                        cur = old_cur;
                        if(offset != 0)
                            --offset;
                        ready = 1;
                        break;
                    case B_RIGHT:
                        if(wrap)
                            continue;
                        ++offset;
                        ready = 1;
                        break;
                    case B_SPACE:
                        if(cur < last)
                        {
                            if(wrap)
                            {
                                old_cur = cur;
                                ready = 1;
                                break;
                            }
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
            else
            {
                if(cur < last)
                {
                    if(wrap)
                    {
                        old_cur = cur;
                    }
                    old_cur = cur - 1;
                    offset = 0;
                }
                else
                {
                    tcsetattr(0, TCSANOW, &old_term);
                    fwprintf(stream, L"\n");
                    return 0;
                }
            }
        }
    }
    return 0;
}


int get_command(wchar_t** buf, wchar_t** cur)
{
    wchar_t w1[MAXWORD], w2[MAXWORD];
    int err;
    int ret = 0;
    if(isatty(0))
        fputws(L"editor: ", stdout);
    if((err = _get_sentence(buf)))
    {
        if(err == E_COMM || err == E_EMPTY)
        {
            if(isatty(0))
                fputwc(L'\n', stdout);
            return get_command(buf, cur);
        }
        return err;
    }
    *cur = *buf;
    if((err = get_word(cur, w1)))
        return err;
    if(wcscmp(w1, L"exit") == 0)
        if(get_word(cur, w2) == 0 && wcscmp(w2, L"force") == 0)
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
    if(ret != 0)
        return ret;
    if((err = get_word(cur, w2)))
        return E_C_WRNG;
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
    return ret;
}

int c_read(line* end, FILE* stream, size_t* total)
{
    size_t add;
    *total = 0;
    while(1)
    {
        wchar_t* s;
        size_t buf = 4, len = 0;
        wint_t c;
        int err;
        s = calloc(1, 4 * sizeof(wchar_t));
        if(s == NULL)
        {
            fclose(stream);
            return E_MALLOC;
        }
        while((c = fgetwc(stream)) != WEOF && c != L'\n')
        {
            if(len + 1 == buf)
            {
                buf = _expand_array((void**)&s, buf, sizeof(wchar_t));
                if(len + 1 == buf)
                {
                    free(s);
                    fclose(stream);
                    return E_MALLOC;
                }
            }
            s[len++] = c;
        }
        s[len] = L'\0';
        err = insert_after(end->prev, s, &add);
        free(s);
        if(err)
        {
            fclose(stream);
            return err;
        }
        *total += add;
        if(c == WEOF)
        {
            break;
        }

    }
    fclose(stream);
    return 0;
}

int c_write(line* begin, line* end, FILE* stream)
{
    line* cur = begin->next;
    int err;
    while(cur != end)
    {
        if((err = fwprintf(stream, L"%ls\n", cur->s)) == -1)
        {
            fclose(stream);
            return E_IOFAIL;
        }
        cur = cur->next;
    }
    fclose(stream);
    return 0;
}

int inv_set_tabwidth(wchar_t** cur)
{
    int err;
    size_t tab;
    if((err = get_int(cur, &tab)))
        return err;
    tab_width = tab;
    return 0;
}

int inv_set_numbers(wchar_t** cur, char* num)
{
    int err;
    wchar_t ans[MAXWORD];
    if((err = get_word(cur, ans)))
        return E_NOARG;
    if(wcscmp(ans, L"yes") == 0)
        *num = 1;
    else if(wcscmp(ans, L"no") == 0)
        *num = 0;
    else
        return E_NOARG;
    return 0;
}

int inv_print_pages(line* begin, line* end, size_t len,
                    char num, char wrap)
{
    int err;
    if((err = print_pages(begin, end, 1, len + 1, stdout, num, wrap)))
        return err;
    return 0;
}

int inv_print_range(wchar_t** cur,
                    line* begin, line* end, size_t len,
                    char num, char wrap)
{
    int err;
    size_t first = 1, last = len;
    get_int(cur, &first);
    get_int(cur, &last);
    ++last;
    first = min(first, len + 1);
    last = min(last, len + 1);
    if((err = print_pages(begin, end, first, last, stdout, num, wrap)))
        return err;
    return 0;

}

int inv_set_wrap(wchar_t** cur, char* wrap)
{
    int err;
    wchar_t ans[MAXWORD];
    if((err = get_word(cur, ans)))
        return E_NOARG;
    if(wcscmp(ans, L"yes") == 0)
        *wrap = 1;
    else if(wcscmp(ans, L"no") == 0)
        *wrap = 0;
    else
        return E_NOARG;
    return 0;
}

int inv_insert_after(wchar_t** cur,
                     line* begin, line* end, size_t* len)
{
    int err;
    size_t total;
    size_t ln;
    line* l;
    wchar_t *s = NULL;
    if((err = get_int(cur, &ln)))
        ln = *len;
    if(ln > *len)
        ln = *len;
    if((err = get_triple_quoted_string(cur, &s)))
        return err;
    l = _find_line(begin, end, ln);
    if(l == NULL)
        return E_LNRNG;
    if((err = insert_after(l, s, &total)))
        return err;
    free(s);
    *len += total;
    return 0;
}

int inv_edit_string(wchar_t** cur, line* begin, line* end, int mode)
{
    int err;
    wchar_t c;
    size_t ln, pos;
    line* l;
    if((err = get_int(cur, &ln)))
        return err;
    if((err = get_int(cur, &pos)))
        return err;
    --pos;
    if((err = get_symb(cur, &c, 1)))
        return err;
    l = _find_line(begin, end, ln);
    if(l == end)
        return E_LNRNG;
    if((err = (mode == M_INSERT ? insert_symbol(l, pos, c) : edit_string(l, pos, c))))
        return err;
    return 0;
}

int inv_replace_substring(wchar_t** cur, line* begin, line* end, size_t* len)
{
    int err;
    wchar_t *sample, *repl;
    size_t total;
    size_t first = 1, last = *len;
    get_int(cur, &first);
    get_int(cur, &last);
    ++last;
    if(first > (*len) + 1)
        first = (*len) + 1;
    if(last > (*len) + 1)
        last = (*len) + 1;
    if((err = get_quoted_string(cur, &sample)))
        return err;
    if((err = get_quoted_string(cur, &repl)))
    {
        free(sample);
        return err;
    }
    if((err = replace_substring(begin, end, first, last, sample, repl, &total)))
    {
        free(sample);
        free(repl);
        return err;
    }
    free(sample);
    free(repl);
    *len += total;
    return 0;
}

int inv_delete_range(wchar_t** cur, line* begin, line* end, size_t* len)
{
    int err;
    size_t first = 1, last = *len;
    size_t total;
    get_int(cur, &first);
    get_int(cur, &last);
    ++last;
    first = min(first, *len + 1);
    last = min(last, *len + 1);
    if((err = delete_range(begin, end, first, last, &total)))
        return err;
    *len -= total;
    return 0;
}

int inv_delete_braces(wchar_t** cur, line* begin, line* end, size_t* len)
{
    int err;
    size_t first = 1, last = *len;
    size_t total;
    get_int(cur, &first);
    get_int(cur, &last);
    ++last;
    first = min(first, *len + 1);
    last = min(last, *len + 1);
    if((err = delete_braces(begin, end, first, last, &total)))
        return err;
    *len -= total;
    return 0;
}

int inv_exit(line* begin, line* end, char* path, wchar_t* buf, char change)
{
    size_t trash;
    if(change)
        return E_NOTSAV;
    delete_range(begin, end, 1, -1, &trash);
    __line_destroy(begin);
    __line_destroy(end);
    if(path != NULL)
        free(path);
    if(buf != NULL)
        free(buf);
    exit(0);
}

int inv_read(wchar_t** cur, line* end, size_t* len)
{
    int err;
    wchar_t *p;
    size_t l;
    char* pc;
    FILE* f;
    if((err = get_quoted_string(cur, &p)))
        return err;
    l = wcstombs(NULL, p, 0);
    pc = calloc(1, l + 1);
    if(pc == NULL)
    {
        free(p);
        return E_MALLOC;
    }
    wcstombs(pc, p, l + 1);
    f = fopen(pc, "r");
    if(f == NULL)
    {
        free(p);
        free(pc);
        return E_IOFAIL;
    }
    if((err = c_read(end, f, len)))
    {
        free(p);
        free(pc);
        return err;
    }
    free(p);
    free(pc);
    return 0;
}

int inv_open(wchar_t** cur, line* end, size_t* len, char** path)
{
    int err;
    wchar_t *p;
    size_t l;
    char* pc;
    FILE* f;
    if((err = get_quoted_string(cur, &p)))
        return err;
    l = wcstombs(NULL, p, 0);
    pc = calloc(1, l + 1);
    if(pc == NULL)
    {
        free(p);
        return E_MALLOC;
    }
    wcstombs(pc, p, l + 1);
    f = fopen(pc, "r");
    if(f == NULL)
    {
        free(p);
        free(pc);
        return E_IOFAIL;
    }
    else
    {
        free(*path);
        strcpy(pc, *path);
    }
    if((err = c_read(end, f, len)))
    {
        free(p);
        free(pc);
        return err;
    }
    free(p);
    free(pc);
    return 0;
}

int inv_write(wchar_t** cur, line* begin, line* end, char* path)
{
    int err;
    wchar_t *p = NULL;
    size_t l;
    char* pc;
    FILE* f;
    if((err = get_quoted_string(cur, &p)))
    {
        if(path[0] == '\0')
            return E_NOFILE;
        pc = path;
    }
    else
    {
        l = wcstombs(NULL, p, 0);
        pc = calloc(1, l + 1);
        if(pc == NULL)
        {
            free(p);
            return E_MALLOC;
        }
        wcstombs(pc, p, l + 1);
    }
    f = fopen(pc, "w+");
    if(f == NULL)
    {
        if(p != NULL)
            free(p);
        if(pc != path)
            free(pc);
        return E_IOFAIL;
    }
    if((err = c_write(begin, end, f)))
    {
        if(p != NULL)
            free(p);
        if(pc != path)
            free(pc);
        return err;
    }
    if(p != NULL)
        free(p);
    if(pc != path)
        free(pc);
    return 0;
}

int inv_set_name(wchar_t** cur, char** path)
{
    int err;
    wchar_t *p;
    size_t l;
    if((err = get_quoted_string(cur, &p)))
        return err;
    free(*path);
    l = wcstombs(NULL, p, 0);
    *path = calloc(1, l + 1);
    wcstombs(*path, p, l + 1);
    free(p);
    return 0;
}

int printerr(int err)
{
    switch(err)
    {
    case 0:
        break;
    case E_STRRNG:
        fwprintf(stderr, L"Incorrect symbol position\n");
        break;
    case E_MALLOC:
        fwprintf(stderr, L"Not enough memory\n");
        break;
    case E_LNRNG:
        fwprintf(stderr, L"Incorrect line number\n");
        break;
    case E_C_WRNG:
        fwprintf(stderr, L"Incorrect command\n");
        break;
    case E_NOARG:
        fwprintf(stderr, L"Missing or invalid arguments\n");
        break;
    case E_NARROW:
        fwprintf(stderr, L"Treminal\'s width not enough\n");
        break;
    case E_IOFAIL:
        fwprintf(stderr, L"Input/output error has occured\n");
        break;
    case E_NOFILE:
        fwprintf(stderr, L"No file specified\n");
        break;
    case E_NOTSAV:
        wprintf(L"File not saved. Use %sexit force%s to exit anyway.\n", RED, RESET);
        break;
    default:
        fwprintf(stderr, L"What the...");
    }
    return err;
}

int main(int argc, const char *argv[])
{
    line *begin, *end;
    char *path = NULL;
    size_t len = 0;
    char change = 0, wrap = 1, num = 1;
    setlocale(LC_ALL, "ru_RU.utf8");
    begin = calloc(1, sizeof(line));
    end = calloc(1, sizeof(line));
    __line_init(begin);
    __line_init(end);
    begin->next = end;
    end->prev = begin;

    if(argc > 1)
    {
        FILE* f;
        path = calloc(1, (strlen(argv[1]) + 1) * sizeof(char));
        if(path == NULL)
            return printerr(E_MALLOC);
        strcpy(path, argv[1]);
        f = fopen(path, "r");
        if(f == NULL)
            printerr(E_IOFAIL);
        else
        {
            c_read(end, f, &len);
        }
    }
    else
    {
        path = calloc(1, sizeof(char));
        if(path == NULL)
            return printerr(E_MALLOC);
        path[0] = '\0';
    }
    while(1)
    {
        wchar_t *buf, *cur;
        int cmd = get_command(&buf, &cur);
        switch(cmd)
        {
        case E_EOF:
            inv_exit(begin, end, path, buf, 0);
            break;
        case E_C_WRNG:
            printerr(cmd);
            break;
        case C_SET_T:
            printerr(inv_set_tabwidth(&cur));
            break;
        case C_SET_N:
            printerr(inv_set_numbers(&cur, &num));
            break;
        case C_PRINT_P:
            printerr(inv_print_pages(begin, end, len, num, wrap));
            break;
        case C_PRINT_R:
            printerr(inv_print_range(&cur, begin, end, len, num, wrap));
            break;
        case C_SET_W:
            printerr(inv_set_wrap(&cur, &wrap));
            break;
        case C_INSERT_A:
            printerr(inv_insert_after(&cur, begin, end, &len));
            change = 1;
            break;
        case C_EDIT_S:
            printerr(inv_edit_string(&cur, begin, end, M_EDIT));
            change = 1;
            break;
        case C_INSERT_S:
            printerr(inv_edit_string(&cur, begin, end, M_INSERT));
            change = 1;
            break;
        case C_REPL_S:
            printerr(inv_replace_substring(&cur, begin, end, &len));
            change = 1;
            break;
        case C_DELETE_R:
            printerr(inv_delete_range(&cur, begin, end, &len));
            change = 1;
            break;
        case C_DELETE_B:
            printerr(inv_delete_braces(&cur, begin, end, &len));
            change = 1;
            break;
        case C_EXIT:
            printerr(inv_exit(begin, end, path, buf, change));
            break;
        case C_EXIT_F:
            printerr(inv_exit(begin, end, path, buf, 0));
            break;
        case C_READ:
            printerr(inv_read(&cur, end, &len));
            break;
        case C_OPEN:
            printerr(inv_open(&cur, end, &len, &path));
            break;
        case C_WRITE:
            printerr(inv_write(&cur, begin, end, path));
            change = 0;
            break;
        case C_SET_NM:
            printerr(inv_set_name(&cur, &path));
            break;
        case C_HELP:
            break;
        }
        free(buf);
    }
    return 0;
}
