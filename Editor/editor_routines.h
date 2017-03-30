/**
  * This header contains definitions for functions
  * representing actual editor commands to be handled by invokers
  */






#ifndef EDITOR_ROUTINES_H
#define EDITOR_ROUTINES_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif /*_XOPEN_SOURCE*/

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>

#include "editor_err.h"
#include "editor_line.h"
#include "editor_utils.h"
#include "editor_getters.h"


int insert_after(line *l, const wchar_t *s, size_t *total);

int edit_string(line *l, size_t pos, wchar_t symb);

int insert_symbol(line *l, size_t pos, wchar_t symb);

int delete_range(line *begin, line *end, size_t first, size_t last, size_t *total);

int delete_braces(line *begin, line *end, size_t first, size_t last, size_t *total);

int replace_substring(line* begin, line* end, size_t first, size_t last,
                      const wchar_t* sample, const wchar_t* repl, size_t* total);

int print_pages(line* begin, line* end, size_t first, size_t last, FILE* stream, char num, char wrap);

int c_read(line* end, FILE* stream, size_t* total);

int c_write(line* begin, line* end, FILE* stream);

#endif /*EDITOR_ROUTINES_H*/