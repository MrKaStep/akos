#ifndef PROGRAM_H
#define PROGRAM_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include<stdlib.h>

#include "errors.h"

#define M_REWRITE 0
#define M_APPEND  1

#define R_EOLN 1
#define R_BACK 2
#define R_FORE 3

struct program_s
{
    char* name;
    int number_of_arguments;
    char** arguments;
    char *input_file, *output_file;
    int output_type;
};

typedef struct program_s program;

void program_init(program* p);

void program_destroy(program* p);

void print_program(program* p);

#endif /*PROGRAM_H*/