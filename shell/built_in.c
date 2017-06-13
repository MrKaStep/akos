#include "built_in.h"

int is_internal(char* name)
{
    if(!strcmp(name, "cd"))
        return INT_CD;
    if(!strcmp(name, "pwd"))
        return INT_PWD;
    if(!strcmp(name, "exit"))
        return INT_EXIT;
    if(!strcmp(name, "bg"))
        return INT_BG;
    if(!strcmp(name, "fg"))
        return INT_FG;
    if(!strcmp(name, "jobs"))
        return INT_JOBS;
    return 0;
}

int cd(program* p)
{
    char* path;
    if(p->number_of_arguments < 2)
    {
        path = getenv("HOME");
    }
    else
    {
        path = p->arguments[1];
    }
    if(chdir(path) == -1)
    {
        perror("cd");
        return E_CHDIR;
    }
    getwd(getenv("PWD"));
    return E_OK;
}

int pwd()
{
    puts(getenv("PWD"));
    return E_OK;
}

void print_jobs()
{
    size_t i;
    for(i = 0; i < jobs_count; ++i)
    {
        if(!jobs[i]->printed)
            print_job(jobs[i]);
        if(jobs[i]->complete)
            jobs[i]->printed = 1;
    }
    if(jobs_count > jobs_alive * 2)
        refine_jobs();
}