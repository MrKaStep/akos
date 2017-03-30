/**
  * Following group of functions invokes commands
  * recognized by get_command.
  * They parse arguments from buffer using get_ functions and
  * invoke them after checking all arguments
  */

#ifndef EDITOR_INVOKERS_H
#define EDITOR_INVOKERS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif /*_XOPEN_SOURCE*/


#include <stdio.h>
#include <wchar.h>

#include "editor_line.h"
#include "editor_err.h"
#include "editor_utils.h"
#include "editor_routines.h"

extern size_t tab_width;

int inv_set_tabwidth(wchar_t** cur);


int inv_set_numbers(wchar_t** cur, char* num);


int inv_print_pages(line* begin, line* end, size_t len,
                    char num, char wrap);


int inv_print_range(wchar_t** cur,
                    line* begin, line* end, size_t len,
                    char num, char wrap);


int inv_set_wrap(wchar_t** cur, char* wrap);


int inv_insert_after(wchar_t** cur,
                     line* begin, line* end, size_t* len);


int inv_edit_string(wchar_t** cur, line* begin, line* end, int mode);


int invline_replace_substring(wchar_t** cur, line* begin, line* end, size_t* len);


int inv_delete_range(wchar_t** cur, line* begin, line* end, size_t* len);


int inv_delete_braces(wchar_t** cur, line* begin, line* end, size_t* len);


int inv_exit(line* begin, line* end, char* path, wchar_t* buf, char change);


int inv_read(wchar_t** cur, line* end, size_t* len);


int inv_open(wchar_t** cur, line* end, size_t* len, char** path);


int inv_write(wchar_t** cur, line* begin, line* end, char* path);


int inv_set_name(wchar_t** cur, char** path);

#endif /*EDITOR_INVOKERS_H*/