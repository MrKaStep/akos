/**
  * This header contains definitions for utility functions used
  * in all project
  */



#ifndef EDITOR_UTILS_H
#define EDITOR_UTILS_H

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

typedef unsigned int ui32;

extern size_t tab_width;

size_t min(size_t a, size_t b);

size_t expand_array(void **s, size_t len, size_t item);

wchar_t wcscat_at(const wchar_t *str1, size_t len1, const wchar_t *str2, size_t pos);

int wcscat_z_function(size_t **z, const wchar_t *str1, size_t len1, const wchar_t *str2, size_t len2);

int resize_line(line *l);

int refine_line(line *l, size_t* total);

void internal_delete_line(line *l);

int merge_lines(line *left, line *right, size_t rstart, size_t *add);

line* find_line(line *begin, line *end, size_t index);

int line_replace_substring(line *l, const wchar_t *sample, const wchar_t *repl, size_t* total);

int print_line(line* l, char mode, size_t width, size_t offset, FILE* stream,...);

#endif /*EDITOR_UTILS_H*/