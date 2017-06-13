#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "core.h"
#include "program.h"
#include "getters.h"
#include "built_in.h"

int main(int argc, char* argv[])
{
    _argc = argc;
    _argv = argv;
    init();
    while(1)
    {
        job* j;
        if(get_job(&j) == E_UNEOLN)
        {
            puts("");
            inv_exit(0);
        }
        else
        {
            if(j->number_of_programs == 0)
            {
                job_destroy(j);
                continue;
            }
            if(j->number_of_programs == 1)
            {
                if(is_internal(j->programs[0]->name))
                {
                    program* p = j->programs[0];
                    j->programs[0] = NULL;
                    job_destroy(j);
                    handle_internal(p, 0);
                    program_destroy(p);
                    continue;
                }
            }
            handle_job(j);
        }
    }


    inv_exit(1);
    return 0;
}