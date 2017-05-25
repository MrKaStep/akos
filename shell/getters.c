#include "getters.h"

void invite_user()
{
    if(isatty(1)) {
        set_color(GREEN);
        fprintf(stdout, "%s@%s ", user->pw_name, hostname);
        set_color(BLUE);
        print_path(0);
        printf(" $ ");
        set_color(RESET);
    }
}

int get_command()
{
    size_t len = 0;
    invite_user();
    buf = calloc(16, sizeof(char));
    if(buf == NULL) {
        return E_MALLOC;
    }
    buf_size = 16;
    while(1) {
        size_t l, new_size;
        if(fgets(buf + len, buf_size - len - 1, stdin) == NULL)
            break;
        l = strlen(buf + len);
        if(buf[len + l - 1] == '\n')
            break;
        len += l;
        new_size = expand_array((void**)&buf, buf_size, sizeof(char));
        if(new_size == buf_size) {
            free(*buf);
            return E_MALLOC;
        }
        buf_size = new_size;
    }
    return E_OK;
}

void skip_spaces()
{
    while(isspace(*cur))
        ++cur;
}

int get_token(char** dest)
{
    char* start = cur;
    int quoted = 0;
    int l;
    skip_spaces();
    if(*cur == '\0')
        return E_EOLN;
    if(*cur == '"') {
        quoted = 1;
    }
    while(*cur != '\0' && (quoted ? *cur != '"' : !isspace(*cur)))
        ++cur;
    l = cur - start;
    *dest = calloc(l + 1, sizeof(char));
    memcpy(*dest, start, l * sizeof(char));
    return E_OK;
}

int get_program(program** p)
{
    *p = calloc(1, sizeof(program));
    char* tok;
    get_token(&(*p)->name);
    
}


int get_job(job** j)
{
    program* programs;

}