#ifndef PROGRAM_H
#define PROGRAM_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include "errors.h"

struct program_s
{
    char* name;
    int number_of_arguments;
    char** arguments;
    char *input_file, *output_file;
    int output_type;
};

typedef struct program_s program;

#endif /*PROGRAM_H*/