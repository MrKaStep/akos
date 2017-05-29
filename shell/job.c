#include "job.h"

void job_destroy(job* j)
{
    int i;
    for(i = 0; i < j->number_of_programs; ++i)
    {
        program_destroy(j->programs[i]);
    }
    free(j->programs);
    free(j);
}

void print_job(job* j)
{
    int i;
    printf("Programs: %d\n Background: %s\n Programs:\n",
           j->number_of_programs,
           j->background ? "yes" : "no");
    for(i = 0; i < j->number_of_programs; ++i)
    {
        puts("-------------------");
        print_program(j->programs[i]);
    }
}