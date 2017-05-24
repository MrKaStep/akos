#include "core.h"

int init()
{
    uid_t uid = getuid();
    user = getpwuid(uid);
    if(user == NULL) {
        perror("User unrecognised");
        return E_USR;
    }
    username = user->pw_name;
    home_dir = user->pw_dir;
    hostname = calloc(64, sizeof(char));
    if(hostname == NULL) {
        print_err(E_MALLOC);
    }
    if(gethostname(hostname, 64) == -1) {
        perror("Hostname failure");
    }
    getcwd(cur_path, PATH_MAX);
    return E_OK;
}