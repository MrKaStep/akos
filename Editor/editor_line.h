#ifndef EDITOR_LINE_H
#define EDITOR_LINE_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif /*_XOPEN_SOURCE*/

#include <stdlib.h>
#include <wchar.h>

#include "editor_err.h"

struct lines
{
    wchar_t *s;
    size_t len;
    size_t buf;
    struct lines *prev, *next;
};

typedef struct lines line;

size_t pw2(size_t a);

/**
  * basic line constructor
  */

int line_struct_line_init(line *l);


/**
  * constructs line from string
  */

int line_struct_line_init_str(line *l, const wchar_t *s, int len);


/**
  * destroys line, deallocating memory for string and for structure itself
  */

void line_struct_line_destroy(line *l);

#endif /*EDITOR_LINE_H*/