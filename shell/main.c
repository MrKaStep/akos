#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"





int main(int argc, const char* argv[])
{
    init();
    get_command();
    return 0;
}