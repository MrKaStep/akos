#include "utils.h"

void set_color(char* color)
{
    printf("%s", color);
}

void print_path(int full_path)
{
    char* to_print = getenv("PWD");
    size_t lcur = strlen(to_print), lhom = strlen(getenv("HOME"));
    if(!full_path && lcur >= lhom && strncmp(getenv("PWD"), getenv("HOME"), lhom) == 0)
    {
        printf("~");
        to_print += lhom;
    }
    printf("%s", to_print);
}

size_t expand_array(void **s, size_t len, size_t item)
{
    size_t add = (len > 0 ? len : 1);
    void *ans;
    while (add > 0 && (ans = realloc(*s, (len + add) * item)) == NULL)
    {
        add /= 2;
    }
    if (add > 0)
        *s = ans;
    return add;
}

void refine_jobs()
{
    int i, cur;
    for(i = 0, cur = 0; i < jobs_count; ++i)
    {
        if(!jobs[i]->printed)
        {
            jobs[cur++] = jobs[i];
        }
        else
        {
            job_destroy(jobs[i]);
        }
    }
    jobs = realloc(jobs, (max(cur, 1) + 1) * sizeof(job*));
    jobs_capacity = cur + 1;
    jobs_count = jobs_alive = cur;
}