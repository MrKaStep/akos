#include "built_in.h"

int cd(char* path)
{
    if(chdir(path) == -1)
    {
        perror("Can't change directory");
        return E_CHDIR;
    }
    strcpy(cur_path, path);
    return 0;
}