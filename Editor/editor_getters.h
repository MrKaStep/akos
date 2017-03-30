#ifndef EDITOR_GETTERS_H
#define EDITOR_GETTERS_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif /*_XOPEN_SOURCE*/

#include <stdio.h>
#include <wchar.h>
#include <unistd.h>

#include "editor_err.h"
#include "editor_utils.h"

#define B_ANY   0
#define B_LEFT  1
#define B_RIGHT 2
#define B_SPACE 3
#define B_Q     4

#define C_SET_T    101  /*set tabwidth*/
#define C_SET_N    102  /*set numbers*/
#define C_PRINT_P  103  /*print pages*/
#define C_PRINT_R  104  /*print range*/
#define C_SET_W    105  /*set wrap*/
#define C_INSERT_A 106  /*insert after*/
#define C_EDIT_S   107  /*edit string*/
#define C_INSERT_S 108  /*insert symbol*/
#define C_REPL_S   109  /*replace substring*/
#define C_DELETE_R 110  /*delete range*/
#define C_DELETE_B 111  /*delete braces*/
#define C_EXIT     112  /*exit*/
#define C_EXIT_F   113  /*exit force*/
#define C_READ     114  /*read*/
#define C_OPEN     115  /*open*/
#define C_WRITE    116  /*write*/
#define C_SET_NM   117  /*set name*/
#define C_HELP     118  /*help*/

#define MAXWORD 15

/**
  * gets command specified by user and puts it at _buf address
  * if no triple quotes met, reads to the end-of-line
  * otherwise, reads to the end of line containing closing triple quotes
  */

int get_sentence(wchar_t** _buf);


/**
  * reads next word (sequence of letters) from string specified by _buf
  * and puts it at address s
  * If first non-space symbol is not a letter, returns E_C_WRNG
  * If word length is greater than MAXWORD, returns E_C_WRNG
  */

wchar_t spec_symb(wchar_t symb);

int get_word(wchar_t **_buf, wchar_t* s);


/**
  * reads next single-quoted string (sequence of symbols between nearest and farthest quotes)
  * from string specified by _buf and puts it at address *s
  * If first non-space symbol is not a quote, returns E_NOARG
  */

int get_quoted_string(wchar_t** _buf, wchar_t** s);



/**
  * reads triple quoted string or, if only quoted string specified,
  * reads quoted string from _buf
  */

int get_triple_quoted_string(wchar_t** _buf, wchar_t** s);



/**
  * reads next integer from _buf
  * If first non-space symbol is not a digit, return E_NOARG
  */


int get_int(wchar_t **_buf, size_t* a);



/**
  * reads next non-space symbol from _buf
  * If symbol is preceded by backslash it is treated as if it was single special symbol
  */

int get_symb(wchar_t **buf, wchar_t* c, char spec);



/**
  * returns last pressed button for print pages mode
  * If it is necessary for this mode, appropriate code is returned
  * Otherwise, B_ANY is returned
  */

int get_button();

int get_command(wchar_t** buf, wchar_t** cur);

#endif /*EDITOR_GETTERS_H*/