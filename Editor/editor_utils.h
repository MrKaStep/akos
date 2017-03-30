#ifndef EDITOR_GENERAL_H
#define EDITOR_GENERAL_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif /*_XOPEN_SOURCE*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>

#include "editor_err.h"
#include "editor_line.h"

#define M_WRAP 1
#define M_NUM  2
#define M_TAB  4

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

typedef unsigned int ui32;

extern size_t tab_width;

size_t min(size_t a, size_t b);


/**
  * returns minimal power of two
  * greater or equal than a
  */

size_t pw2(size_t a);


/**
  * attempts to expand array consisting of len items of size item
  * returns number of successfully added items
  */

size_t expand_array(void **s, size_t len, size_t item);


/**
  * returns symbol at position pos from concatenation of str1, null-symbol and str2
  */

wchar_t wcscat_at(const wchar_t *str1, size_t len1, const wchar_t *str2, size_t pos);


/**
  * calculates z-function for concatenation of str1, null-symbol and str2
  */

int _z_function(size_t **z, const wchar_t *str1, size_t len1, const wchar_t *str2, size_t len2);



/**
  * resizes line to pw2(l->len) to support linear amount of memory
  */

int resize_line(line *l);


/**
  * splits line by '\n' symbols into several lines
  * structure is kept correct
  * puts number of resulting line at address total
  */

int refine_line(line *l, size_t* total);



/**
  * deletes line from text
  */

void _delete_line(line *l);



/**
  * collapses two lines into one
  * second line start is specified as rstart
  * everything between the end of left line and rstart at right line is destroyed
  * puts difference between initial and resulting number of line at address add
  */

int merge_lines(line *left, line *right, size_t rstart, size_t *add);



/**
  * return pointer to line in text specified by begin and end pointers
  * at position index
  */

line* find_line(line *begin, line *end, size_t index);

int line_replace_substring(line *l, const wchar_t *sample, const wchar_t *repl, size_t* total);

int print_line(line* l, char mode, size_t width, size_t offset, FILE* stream,...);

#endif /*EDITOR_GENERAL_H*/