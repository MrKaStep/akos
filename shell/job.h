#ifndef JOB_H
#define JOB_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include <stdlib.h>

#include "errors.h"
#include "program.h"

struct job_s
{
    int background;
    program** programs;
    int number_of_programs;

    pid_t pid;

    int stopped;
    int complete;
    int printed;
};

typedef struct job_s job;

void job_destroy(job* j);

void print_job(job* j);

#endif /*JOB_H*/