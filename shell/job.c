#include "job.h"

void job_destroy(job* j)
{
    int i;
    for(i = 0; i < j->number_of_programs; ++i)
    {
        program_destroy(j->programs[i]);
    }
    j->complete = j->stopped = j->printed = 0;
    free(j->programs);
    free(j);
}

void print_job_call(job* j)
{
    size_t i;
    for(i = 0; i < j->number_of_programs; ++i)
    {
        if(i != 0)
            printf("| ");
        print_program(j->programs[i]);
    }
    if(j->background)
        printf("&");
    puts("");
}

void print_job(job* j)
{
    if(j->printed)
        return;
    printf("[%d] %s\t\t", (int)j->pid, j->stopped ? "Stopped" : (j->complete ? "Done" : "Running"));
    print_job_call(j);
}