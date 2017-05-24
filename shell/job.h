#ifndef JOB_H
#define JOB_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1000
#endif

#include "errors.h"

struct job_s
{
    int background;
    struct program* programs;
    int number_of_programs;
};

typedef struct job_s job;

#endif /*JOB_H*/