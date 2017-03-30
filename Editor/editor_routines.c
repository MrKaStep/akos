#include "editor_routines.h"

/**
  * implements isertion of line specified by string s after line l
  * puts number of lines added at address total
  */

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
    refine_line(t, total);
    return 0;
}

/**
  * changes symbol of line l at position pos to symbol symb
  */


int edit_string(line *l, size_t pos, wchar_t symb)
{
    if (pos >= l->len)
    {
        return E_STRRNG;
    }
    l->s[pos] = symb;
    return 0;
}

/**
  * inserts symbol symb to line l after position pos
  */

int insert_symbol(line *l, size_t pos, wchar_t symb)
{
    size_t i;
    if (pos > l->len)
        pos = l->len;
    if (l->len == l->buf - 1)
    {
        if ((l->buf = expand_array((void **)&(l->s), l->buf, sizeof(wchar_t))) - 1 == l->len)
            return E_MALLOC;
    }
    ++l->len;
    for (i = l->len; i > pos; --i)
        l->s[i] = l->s[i - 1];
    l->s[pos] = symb;
    return 0;
}

/**
  * implements line range deleting function
  * puts difference between resulting and initial number of lines at address total
  */

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


/**
  * deletes everything between {...} including braces
  * uses single pass and merge_lines function to delete multistring braces
  * puts difference between resulting and initial number of lines at address total
  */

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
                    if ((err = merge_lines(start, cur, pos + 1, &add)) != 0)
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
        if ((err = resize_line(start)))
            return err;
    }
    return 0;
}

/**
  * runs line_replace_substring for all lines in range [first, last) in text
  * specified by begin and end
  * puts difference between resulting and initial number of lines at address total
  */

int replace_substring(line* begin, line* end, size_t first, size_t last,
                      const wchar_t* sample, const wchar_t* repl, size_t* total)
{
    int err;
    line *l, *nxt;
    size_t cur = first;
    l = find_line(begin, end, first);
    *total = 0;
    if(l == NULL)
        return E_LNRNG;
    while(l != end && cur != last)
    {
        size_t tmp;
        nxt = l->next;
        if((err = line_replace_substring(l, sample, repl, &tmp)))
            return err;
        *total += tmp - 1;
        l = nxt;
    }
    return 0;
}


/**
  * prints text specifed by begin and end pointers
  * from line first inclusive to line last exclusive to stream.
  * Prints each line using print_line and given parameters
  *
  * Supports exiting by pressing 'q', accessing next page by pressing space
  * In no-wrap mode supports left-right scroll using left arrow and right arrow keys
  */



int print_pages(line* begin, line* end, size_t first, size_t last, FILE* stream, char num, char wrap)
{
    size_t cur;
    line* l;
    int err;
    l = find_line(begin, end, first);
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
                if((err = print_line(l, M_NUM, -1, 0, stream, nlen, cur)))
                    return err;
        }
        else
            for(cur = first; cur < last; ++cur, l = l->next)
                if((err = print_line(l, 0, -1, 0, stream)))
                    return err;
        return 0;
    }
    else
    {
        struct termios old_term, new_term;
        struct winsize sz;
        size_t height, width, offset = 0, cur_height, old_cur;
        if(isatty(0))
        {
            tcgetattr(0, &old_term);
            memcpy(&new_term, &old_term, sizeof(struct termios));
            new_term.c_lflag &= ~ECHO;
            new_term.c_lflag &= ~ICANON;
            new_term.c_cc[VMIN] = 1;
            tcsetattr(0, TCSANOW, &new_term);
        }
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
            l = find_line(begin, end, cur);
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
                        if((err = print_line(l, M_NUM | M_WRAP | M_TAB, width, offset, stream, nlen, cur, &offset, &cur_height)))
                        {
                            if(isatty(0))
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
                        if((err = print_line(l, M_NUM | M_TAB, width, offset, stream, nlen, cur)))
                        {
                            if(isatty(0))
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
                        if((err = print_line(l, M_WRAP | M_TAB, width, offset, stream, &offset, &cur_height)))
                        {
                            if(isatty(0))
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
                        if((err = print_line(l, M_TAB, width, offset, stream)))
                        {
                            if(isatty(0))
                                tcsetattr(0, TCSANOW, &old_term);
                            return err;
                        }
                        ++cur;
                        l = l->next;
                        --cur_height;
                    }
                }
            }
            if(isatty(0))
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
                    fwprintf(stream, L"\n");
                    return 0;
                }
            }
        }
    }
    return 0;
}

/**
  * Reads text from file specified by stream
  * and append it do the end of text specified by end pointer
  * Total number of lines is put at address total
  */

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
                buf = expand_array((void**)&s, buf, sizeof(wchar_t));
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


/**
  * prints raw text to file specified by stream
  */

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