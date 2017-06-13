#include "program.h"

void program_init(program* p)
{
    p->name = NULL;
    p->arguments = NULL;
    p->number_of_arguments = 0;
    p->input_file = NULL;
    p->output_file = NULL;
    p->output_type = M_REWRITE;
}

void program_destroy(program* p)
{
    int i;
    if(p->arguments != NULL)
    {
        for(i = 0; i < p->number_of_arguments; ++i)
        {
            free(p->arguments[i]);
        }
        free(p->arguments);
    }
    if(p->input_file != NULL)
    {
        free(p->input_file);
    }
    if(p->output_file != NULL)
    {
        free(p->output_file);
    }
    free(p);
}

void print_program(program* p)
{
    int i;
    for(i = 0; i < p->number_of_arguments; ++i)
    {
        printf("%s ", p->arguments[i]);
    }
    if(p->input_file != NULL)
    {
        printf("< %s ", p->input_file);
    }
    if(p->output_file != NULL)
    {
        printf("%s %s", p->output_type == M_APPEND ? ">>" : ">", p->output_file);
    }
}