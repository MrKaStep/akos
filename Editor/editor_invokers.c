#include "editor_invokers.h"

#include "editor_getters.h"
#include "editor_routines.h"

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
    l = find_line(begin, end, ln);
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
    l = find_line(begin, end, ln);
    if(l == end)
        return E_LNRNG;
    if((err = (mode == M_INSERT ? insert_symbol(l, pos, c) : edit_string(l, pos, c))))
        return err;
    return 0;
}

int invline_replace_substring(wchar_t** cur, line* begin, line* end, size_t* len)
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
    line_struct_line_destroy(begin);
    line_struct_line_destroy(end);
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

int inv_help()
{
    line *begin, *end;
    size_t len;
    FILE* f;
    int err = 0;
    begin = calloc(1, sizeof(line));
    end = calloc(1, sizeof(line));
    line_struct_line_init(begin);
    line_struct_line_init(end);
    end->prev = begin;
    begin->next = end;
    f = fopen(".help", "r");
    if(f == NULL)
    {
        free(begin);
        free(end);
        return E_IOFAIL;
    }
    if((err = c_read(end, f, &len)))
    {
        free(begin);
        free(end);
        return err;
    }
    err = print_pages(begin, end, 1, len + 1, stdout, 0, 0);
    delete_range(begin, end, 1, len + 1,&len);
    line_struct_line_destroy(begin);
    line_struct_line_destroy(end);
    return err;
}