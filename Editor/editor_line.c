#include "editor_line.h"



/**
  * returns minimal power of two
  * greater or equal than a
  */

size_t pw2(size_t a)
{
    size_t t = 1;
    while (t < a)
    {
        t <<= 1;
    }
    return t;
}


/**
  * basic line constructor
  */

int line_struct_line_init(line *l)
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

/**
  * constructs line from string
  */

int line_struct_line_init_str(line *l, const wchar_t *s, int len)
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

/**
  * destroys line, deallocating memory for string and for structure itself
  */

void line_struct_line_destroy(line *l)
{
    free(l->s);
    free(l);
}