#define _XOPEN_SOURCE 1000

#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>

#include <linux/limits.h>

#include "editor_err.h"
#include "editor_utils.h"
#include "editor_getters.h"
#include "editor_invokers.h"
#include "editor_line.h"

size_t tab_width = 8;

int main(int argc, const char *argv[])
{
    line *begin, *end;
    char *path = NULL;
    size_t len = 0;
    char change = 0, wrap = 1, num = 1;
    setlocale(LC_ALL, "");
    begin = calloc(1, sizeof(line));
    end = calloc(1, sizeof(line));
    line_struct_line_init(begin);
    line_struct_line_init(end);
    begin->next = end;
    end->prev = begin;

    if(argc > 1)
    {
        FILE* f;
        path = calloc(1, (strlen(argv[1]) + 1) * sizeof(char));
        if(path == NULL)
            return printerr(E_MALLOC);
        strcpy(path, argv[1]);
        f = fopen(path, "r");
        if(f == NULL)
            printerr(E_IOFAIL);
        else
        {
            c_read(end, f, &len);
        }
    }
    else
    {
        path = calloc(1, sizeof(char));
        if(path == NULL)
            return printerr(E_MALLOC);
        path[0] = '\0';
    }
    while(1)
    {
        wchar_t *buf, *cur;
        int cmd = get_command(&buf, &cur);
        switch(cmd)
        {
        case E_EOF:
            printerr(E_EOF);
            inv_exit(begin, end, path, buf, 0);
            break;
        case E_C_WRNG:
            printerr(cmd);
            break;
        case C_SET_T:
            printerr(inv_set_tabwidth(&cur));
            break;
        case C_SET_N:
            printerr(inv_set_numbers(&cur, &num));
            break;
        case C_PRINT_P:
            printerr(inv_print_pages(begin, end, len, num, wrap));
            break;
        case C_PRINT_R:
            printerr(inv_print_range(&cur, begin, end, len, num, wrap));
            break;
        case C_SET_W:
            printerr(inv_set_wrap(&cur, &wrap));
            break;
        case C_INSERT_A:
            printerr(inv_insert_after(&cur, begin, end, &len));
            change = 1;
            break;
        case C_EDIT_S:
            printerr(inv_edit_string(&cur, begin, end, M_EDIT));
            change = 1;
            break;
        case C_INSERT_S:
            printerr(inv_edit_string(&cur, begin, end, M_INSERT));
            change = 1;
            break;
        case C_REPL_S:
            printerr(invline_replace_substring(&cur, begin, end, &len));
            change = 1;
            break;
        case C_DELETE_R:
            printerr(inv_delete_range(&cur, begin, end, &len));
            change = 1;
            break;
        case C_DELETE_B:
            printerr(inv_delete_braces(&cur, begin, end, &len));
            change = 1;
            break;
        case C_EXIT:
            printerr(inv_exit(begin, end, path, buf, change));
            break;
        case C_EXIT_F:
            printerr(inv_exit(begin, end, path, buf, 0));
            break;
        case C_READ:
            printerr(inv_read(&cur, end, &len));
            break;
        case C_OPEN:
            printerr(inv_open(&cur, end, &len, &path));
            break;
        case C_WRITE:
            printerr(inv_write(&cur, begin, end, path));
            change = 0;
            break;
        case C_SET_NM:
            printerr(inv_set_name(&cur, &path));
            break;
        case C_HELP:
            printerr(inv_help());
            break;
        }
        free(buf);
    }
    return 0;
}
