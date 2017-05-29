#include "getters.h"

void invite_user()
{
    if(isatty(1))
    {
        set_color(GREEN);
        fprintf(stdout, "%s@%s ", user->pw_name, hostname);
        set_color(BLUE);
        print_path(0);
        printf(" *$ ");
        set_color(RESET);
    }
}

char spec_symb(char c)
{
    switch(c)
    {
    case 't':
        return '\t';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    default:
        return c;
    }
}

int get_command()
{
    size_t len = 0;
    if(need_invite)
    {
        invite_user();
        need_invite = 0;
    }
    if(buf != NULL)
    {
        free(buf);
    }
    buf = calloc(16, sizeof(char));
    if(buf == NULL)
    {
        return E_MALLOC;
    }
    buf_size = 16;
    while(1)
    {
        int c;
        if(buf_size - 1 == len)
        {
            size_t add = expand_array((void**)&buf, buf_size, sizeof(char));
            if(add == 0)
            {
                free(buf);
                return E_MALLOC;
            }
            buf_size += add;
        }
        c = fgetc(stdin);
        if(c == ';' || c == '\n' || c == EOF)
        {
            buf[len] = '\0';
            cur = buf;
            if(c != ';')
                need_invite = 1;
            return E_OK;
        }
        buf[len++] = c;
    }
}

void skip_spaces()
{
    while(isspace(*cur))
        ++cur;
}

int get_int()
{
    int ans = 0;
    while(isdigit(*cur))
    {
        ans *= 10;
        ans += *(cur++) - '0';
    }
    return ans;
}

int get_var(char** dest)
{
    ++cur;
    if(*cur != '{')
    {
        if(isdigit(*cur))
        {
            int a = get_int();
            if(a > _argc - 1 || a < 0)
            {
                return E_ARGCOF;
            }
            *dest = calloc(strlen(_argv[a]), sizeof(char));
            if(*dest == NULL)
            {
                return E_MALLOC;
            }
            sprintf(*dest, "%s", _argv[a]);
            return E_OK;
        }
        switch(*cur)
        {
        case '?':
            *dest = calloc(16, sizeof(char));
            if(*dest == NULL)
            {
                return E_MALLOC;
            }
            sprintf(*dest, "%d", last_foreground_result);
            ++cur;
            return E_OK;
        case '#':
            *dest = calloc(16, sizeof(char));
            if(*dest == NULL)
            {
                return E_MALLOC;
            }
            sprintf(*dest, "%d", _argc);
            ++cur;
            return E_OK;
        default:
            return E_DOLLAR;
        }
    }
    else
    {
        char* start = ++cur;
        while(*cur != '\0' && *cur != '}')
        {
            ++cur;
        }
        if(*cur == '\0')
            return E_UNEOLN;
        if(cur - start == 4 && strncmp(start, "HOME", 4) == 0)
        {
            *dest = calloc(strlen(home_dir) + 1, sizeof(char));
            if(*dest == NULL)
            {
                return E_MALLOC;
            }
            strcpy(*dest, home_dir);
        }
        else if(cur - start == 3 && strncmp(start, "PWD", 3) == 0)
        {
            *dest = calloc(strlen(cur_path) + 1, sizeof(char));
            if(*dest == NULL)
            {
                return E_MALLOC;
            }
            strcpy(*dest, cur_path);
        }
        else if(cur - start == 3 && strncmp(start, "PID", 3) == 0)
        {
            *dest = calloc(16, sizeof(char));
            if(*dest == NULL)
            {
                return E_MALLOC;
            }
            sprintf(*dest, "%d", shell_pid);
        }
        ++cur;
    }
    return E_OK;
}

int get_token(char** dest, int* md)
{
    int mode;
    char* _buf = calloc(16, sizeof(char));
    int buf_len = 16;
    int len = 0;
    if(_buf == NULL)
    {
        return E_MALLOC;
    }
    skip_spaces();
    if(*cur == '\0')
    {
        free(_buf);
        return R_EOLN;
    }
    switch(*cur)
    {
    case '"':
        mode = M_DQUOT;
        ++cur;
        break;
    case '\'':
        mode = M_QUOT;
        ++cur;
        break;
    default:
        mode = M_NONE;
    }
    *md = mode;
    while
    (
        *cur != '\0' && *cur != '\n' &&
        (
            (mode == M_QUOT && *cur != '\'') ||
            (mode == M_DQUOT && *cur != '"') ||
            (mode == M_NONE && !isspace(*cur))
        )
    )
    {
        char* var;
        if(buf_len - 1 == len)
        {
            int add = expand_array((void**)&_buf, buf_len, sizeof(char));
            if(add == 0)
            {
                free(_buf);
                return E_MALLOC;
            }
            buf_len += add;
        }
        if(*cur == '#' && mode == M_NONE)
        {
            *cur = '\0';
            break;
        }
        if(*cur == '\\')
        {
            ++cur;
            if(*cur == '\0' || *cur == '\n')
            {
                free(_buf);
                return E_UNEOLN;
            }
            _buf[len++] = spec_symb(*(cur++));
            continue;
        }
        if(*cur == '$' && mode != M_QUOT)
        {
            int err;
            int l;
            if((err = get_var(&var)))
            {
                free(_buf);
                return err;
            }
            l = strlen(var);
            while(len + l + 1>= buf_len)
            {
                int add = expand_array((void**)&_buf, buf_len, sizeof(char));
                if(add == 0)
                {
                    free(_buf);
                    return E_MALLOC;
                }
                buf_len += add;
            }
            strcpy(_buf + len, var);
            len += l;
            free(var);
        }
        else
            _buf[len++] = *(cur++);
    }
    if(
        (mode == M_QUOT && *cur != '\'') ||
        (mode == M_DQUOT && *cur != '"'))
    {
        free(_buf);
        return E_UNEOLN;
    }
    if(*cur == '\'' || *cur == '"')
        ++cur;
    _buf[len] = '\0';
    *dest = _buf;
    return E_OK;
}

int get_program(program** _p)
{
    char* tok;
    int mode;
    int err;
    int args_capacity = 2;
    int iter = 0;
    program* p = calloc(1, sizeof(program));
    if(p == NULL)
    {
        return E_MALLOC;
    }
    program_init(p);
    p->arguments = calloc(2, sizeof(char*));
    if(p->arguments == NULL)
    {
        program_destroy(p);
        return E_MALLOC;
    }
    while(++iter, 1)
    {
        if((err = get_token(&tok, &mode)) == R_EOLN)
        {
            if(iter == 1)
            {
                program_destroy(p);
                return E_UNEOLN;
            }
            p->arguments = realloc(p->arguments, (p->number_of_arguments + 1) * sizeof(char**));
            p->arguments[p->number_of_arguments] = NULL;
            p->name = p->arguments[0];
            *_p = p;
            return R_FORE;
        }
        if(err)
        {
            program_destroy(p);
            return err;
        }
        if(strcmp(tok, "|") == 0)
        {
            free(tok);
            if(iter == 0)
            {
                program_destroy(p);
                return E_PIPERR;
            }
            p->arguments = realloc(p->arguments, (p->number_of_arguments + 1) * sizeof(char**));
            p->arguments[p->number_of_arguments] = NULL;
            p->name = p->arguments[0];
            *_p = p;
            return E_OK;
        }
        if(mode == M_NONE && strcmp(tok, "<") == 0)
        {
            free(tok);
            if(p->input_file != NULL)
            {
                program_destroy(p);
                return E_DREDIR;
            }
            if((err = get_token(&tok, &mode)) < 0)
            {
                program_destroy(p);
                return err;
            }
            p->input_file = tok;
            continue;
        }
        if(mode == M_NONE && strcmp(tok, ">") == 0)
        {
            free(tok);
            if(p->output_file != NULL)
            {
                program_destroy(p);
                return E_DREDIR;
            }
            if((err = get_token(&tok, &mode)) < 0)
            {
                program_destroy(p);
                return err;
            }
            p->output_file = tok;
            p->output_type = M_REWRITE;
            continue;
        }
        if(mode == M_NONE && strcmp(tok, ">>") == 0)
        {
            free(tok);
            if(p->output_file != NULL)
            {
                program_destroy(p);
                return E_DREDIR;
            }
            if((err = get_token(&tok, &mode)) < 0)
            {
                program_destroy(p);
                return err;
            }
            p->output_file = tok;
            p->output_type = M_APPEND;
            continue;
        }
        if(mode == M_NONE && strcmp(tok, "&") == 0)
        {
            free(tok);
            p->arguments = realloc(p->arguments, (p->number_of_arguments + 1) * sizeof(char**));
            p->arguments[p->number_of_arguments] = NULL;
            p->name = p->arguments[0];
            *_p = p;
            return R_BACK;
        }
        if(args_capacity == p->number_of_arguments + 1)
        {
            int add;
            if((add = expand_array((void**)&(p->arguments), args_capacity, sizeof(char*))) == 0)
            {
                program_destroy(p);
                return E_MALLOC;
            }
            args_capacity += add;
        }
        p->arguments[p->number_of_arguments++] = tok;
    }
}


int get_job(job** _j)
{
    job* j;
    int prog_capacity = 1;
    int err;
    if((err = get_command()))
    {
        return err;
    }
    j = calloc(1, sizeof(job));
    if(j == NULL)
    {
        return E_MALLOC;
    }
    j->programs = calloc(1, sizeof(program*));
    if(j->programs == NULL)
    {
        free(j);
        return E_MALLOC;
    }
    j->number_of_programs = 0;
    while((err = get_program((j->programs + j->number_of_programs))) == E_OK)
    {
        ++j->number_of_programs;
        if(j->number_of_programs == prog_capacity)
        {
            int add = expand_array((void**)&j->programs, prog_capacity, sizeof(program*));
            if(add == 0)
            {
                job_destroy(j);
                return E_MALLOC;
            }
            prog_capacity += add;
        }
    }
    if(err < 0 && (err != E_UNEOLN || prog_capacity > 1))
    {
        job_destroy(j);
        return err;
    }
    if(err == E_UNEOLN)
    {
        job_destroy(j);
        return get_job(_j);
    }
    ++j->number_of_programs;
    if(err == R_BACK)
    {
        j->background = 1;
    }
    else
    {
        j->background = 0;
    }
    *_j = j;
    return E_OK;
}