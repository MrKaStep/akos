#include "editor_getters.h"

/**
  * gets command specified by user and puts it at _buf address
  * if no triple quotes met, reads to the end-of-line
  * otherwise, reads to the end of line containing closing triple quotes
  */

int get_sentence(wchar_t** _buf)
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
            buf = expand_array((void**)&s, buf, sizeof(wchar_t));
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

/**
  * returns special symbol, as if it was preceded by backslash symbol
  * if specified symbol has no special meaning, return symbol itself
  */

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
    default:
        return symb;
    }
}

/**
  * reads next word (sequence of letters) from string specified by _buf
  * and puts it at address s
  * If first non-space symbol is not a letter, returns E_C_WRNG
  * If word length is greater than MAXWORD, returns E_C_WRNG
  */

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

/**
  * reads next single-quoted string (sequence of symbols between nearest and farthest quotes)
  * from string specified by _buf and puts it at address *s
  * If first non-space symbol is not a quote, returns E_NOARG
  */

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


/**
  * reads triple quoted string or, if only quoted string specified,
  * reads quoted string from _buf
  */

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


/**
  * reads next integer from _buf
  * If first non-space symbol is not a digit, return E_NOARG
  */


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


/**
  * reads next non-space symbol from _buf
  * If symbol is preceded by backslash it is treated as if it was single special symbol
  */

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


/**
  * returns last pressed button for print pages mode
  * If it is necessary for this mode, appropriate code is returned
  * Otherwise, B_ANY is returned
  */

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

/**
  * returns code of command specified by user
  * uses get_sentence function to retrieve user's input
  * and parses it using get_ commands
  *
  * In case of success, command's arguments are still stored in buf
  * Current cursor position is stored in cur
  *
  * If command is wrong, nothing is stored and E_C_WRNG is returned
  */

int get_command(wchar_t** buf, wchar_t** cur)
{
    wchar_t w1[MAXWORD], w2[MAXWORD];
    int err;
    int ret = 0;
    if(isatty(0))
        fputws(L"editor: ", stdout);
    if((err = get_sentence(buf)))
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