#include "editor_utils.h"

size_t min(size_t a, size_t b)
{
    if(a > b)
        return b;
    return a;
}

/**
  * attempts to expand array consisting of len items of size item
  * returns number of successfully added items
  */

size_t expand_array(void **s, size_t len, size_t item)
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

/**
  * returns symbol at position pos from concatenation of str1, null-symbol and str2
  */

wchar_t wcscat_at(const wchar_t *str1, size_t len1, const wchar_t *str2, size_t pos)
{
    if(pos < len1)
        return str1[pos];
    if(pos == len1)
        return L'\0';
    return str2[pos - len1 - 1];
}

/**
  * calculates z-function for concatenation of str1, null-symbol and str2
  */

int wcscat_z_function(size_t **z, const wchar_t *str1, size_t len1, const wchar_t *str2, size_t len2)
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
                wcscat_at(str1, len1, str2, Z[i]) == wcscat_at(str1, len1, str2, i + Z[i]))
            ++Z[i];
        if (i + Z[i] > r)
            l = i, r = i + Z[i];
    }
    return 0;
}


/**
  * resizes line to pw2(l->len) to support linear amount of memory
  */

int resize_line(line *l)
{
    size_t buf = pw2(l->len + 1);
    wchar_t *s = realloc(l->s, buf * sizeof(wchar_t));
    if (s == NULL)
        return E_MALLOC;
    l->buf = buf;
    l->s = s;
    return 0;
}

/**
  * splits line by '\n' symbols into several lines
  * structure is kept correct
  * puts number of resulting line at address total
  */

int refine_line(line *l, size_t* total)
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
                if ((buf = expand_array((void **)&arr, buf, sizeof(size_t))) - 1 == segm)
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
        if ((err = line_struct_line_init_str(x, l->s + arr[i], len)) != 0)
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
    line_struct_line_destroy(l);
    free(arr);
    free(start);
    return 0;
}


/**
  * deletes line from text
  */

void internal_delete_line(line *l)
{
    l->next->prev = l->prev;
    l->prev->next = l->next;
    line_struct_line_destroy(l);
}


/**
  * collapses two lines into one
  * second line start is specified as rstart
  * everything between the end of left line and rstart at right line is destroyed
  * puts difference between initial and resulting number of line at address add
  */

int merge_lines(line *left, line *right, size_t rstart, size_t *add)
{
    size_t buf = pw2(left->len + right->len + 1);
    wchar_t *s = realloc(left->s, buf * sizeof(wchar_t));
    *add = 1;
    while(left->next != right)
    {
        ++*add;
        internal_delete_line(left->next);
    }
    if (s == NULL)
        return E_MALLOC;
    left->s = s;
    left->buf = buf;
    wcsncpy(left->s + right->len, right->s + rstart, right->len - rstart + 1);
    left->next = right->next;
    left->next->prev = left;
    line_struct_line_destroy(right);
    return 0;
}


/**
  * return pointer to line in text specified by begin and end pointers
  * at position index
  */

line* find_line(line *begin, line *end, size_t index)
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

/**
  * prints line l to stream with specified parameters
  * parameter mode is a mask containing modes from list
  ** M_WRAP --- if wrapping is required
  ** M_NUM  --- if line numbers are required
  ** M_TAB  --- if '\t' symbols should be replaced by spaces
  * width parameter specifies maximum line width
  * offset parameter specifies first symbol that has to be printed
  *
  * If M_NUM is specified, two arguments after stream have to be width of line number
  * and line number itself
  *
  * If M_WRAP is specified, pointer to remaining height of terminal has to be the last argument
  * It will be decremented by number of actually printed lines
  */

int print_line(line* l, char mode, size_t width, size_t offset, FILE* stream,...)
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


/**
  * replaces all occurences of sample string with repl string
  * puts number of lines, produced by this replace at address total
  *
  * Uses z-function to find all occurences in linear time
  */


int line_replace_substring(line *l, const wchar_t *sample, const wchar_t *repl, size_t* total)
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
        if((err = refine_line(l, total)))
            return err;
        return 0;
    }


    wcscat_z_function(&z, sample, slen, l->s, l->len);

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
    if((err = refine_line(l, total)))
        return err;
    return 0;
}