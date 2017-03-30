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


/**
  * implements isertion of line specified by string s after line l
  * puts number of lines added at address total
  */

int insert_after(line *l, const wchar_t *s, size_t *total);


/**
  * changes symbol of line l at position pos to symbol symb
  */


int edit_string(line *l, size_t pos, wchar_t symb);


/**
  * inserts symbol symb to line l after position pos
  */

int insert_symbol(line *l, size_t pos, wchar_t symb);


/**
  * implements line range deleting function
  * puts difference between resulting and initial number of lines at address total
  */

int delete_range(line *begin, line *end, size_t first, size_t last, size_t *total);



/**
  * deletes everything between  including braces
  * uses single pass and merge_lines function to delete multistring braces
  * puts difference between resulting and initial number of lines at address total
  */

int delete_braces(line *begin, line *end, size_t first, size_t last, size_t *total);


/**
  * runs line_replace_substring for all lines in range [first, last) in text
  * specified by begin and end
  * puts difference between resulting and initial number of lines at address total
  */

int replace_substring(line* begin, line* end, size_t first, size_t last,
                      const wchar_t* sample, const wchar_t* repl, size_t* total);



/**
  * prints text specifed by begin and end pointers
  * from line first inclusive to line last exclusive to stream.
  * Prints each line using print_line and given parameters
  *
  * Supports exiting by pressing 'q', accessing next page by pressing space
  * In no-wrap mode supports left-right scroll using left arrow and right arrow keys
  */



int print_pages(line* begin, line* end, size_t first, size_t last, FILE* stream, char num, char wrap);


/**
  * Reads text from file specified by stream
  * and append it do the end of text specified by end pointer
  * Total number of lines is put at address total
  */

int c_read(line* end, FILE* stream, size_t* total);



/**
  * prints raw text to file specified by stream
  */

int c_write(line* begin, line* end, FILE* stream);

#endif /*EDITOR_ROUTINES_H*/