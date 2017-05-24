#include "utils.h"

void set_color(char* color)
{
    printf("%s", color);
}

void print_path(int full_path)
{
    char* to_print = cur_path;
    size_t lcur = strlen(cur_path), lhom = strlen(home_dir);
    if(!full_path && lcur >= lhom && strncmp(cur_path, home_dir, lhom) == 0) {
        printf("~");
        to_print += lhom;
    }
    printf("%s", to_print);
}

size_t expand_array(void **s, size_t len, size_t item)
{
    size_t add = (len > 0 ? len : 1);
    void *ans;
    while (add > 0 && (ans = realloc(*s, (len + add) * item)) == NULL) {
        add /= 2;
    }
    if (add > 0)
        *s = ans;
    return add + len;
}