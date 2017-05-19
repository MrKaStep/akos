#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#define E_ARG   1
#define E_OPEN  2
#define E_RANGE 3

const int long_bits = sizeof(long) * 8;

void move_bits(long* a, int p_left, int p_right, int move_size)
{
    long mask_left, mask_right, n_left, n_right;
    mask_left = (1l << (long_bits - p_left + 1)) - (1l << (long_bits - p_left - move_size + 1));
    mask_right = (1l << (long_bits - p_right + 1)) - (1l << (long_bits - p_right - move_size + 1));
    n_left = *a & mask_left;
    n_right = *a & mask_right;
    *a &= ~(mask_left | mask_right);
    *a |= (n_left >> (p_right - p_left));
    *a |= (n_right << (p_right - p_left));
}

void print_2(long a)
{
    int i;
    for(i = long_bits - 1; i >= 0; --i)
    {
        printf("%d", (a & (1l << i)) > 0 ? 1 : 0);
    }
    puts("");
}

int main(int argc, const char* argv[])
{
    int p_left, p_right, move_size;
    int fd;
    long a, mn = LONG_MAX;
    unsigned char zero = 0;
    char s[16];
    if (argc != 5)
    {
        fprintf(stderr, "\tIncorrect number of arguments\n");
        return E_ARG;
    }
    p_left = atoi(argv[1]);
    p_right = atoi(argv[2]);
    move_size = atoi(argv[3]);
    if(p_right - p_left < move_size || move_size < 0 || p_right + move_size - 1 > long_bits)
    {
        fprintf(stderr, "\tIncorrect arguments\n");
        return E_RANGE;
    }
    fd = open(argv[4], O_WRONLY | O_CREAT, 0666);
    if(fd == -1)
    {
        perror("File open failed");
        return E_OPEN;
    }
    while(fgets(s, 15, stdin) != NULL)
    {
        a = strtol(s, NULL, 16);
        print_2(a);
        move_bits(&a, p_left, p_right, move_size);
        print_2(a);
        puts("");
        write(fd, &a, sizeof(long));
        if(labs(a) < mn)
        {
            mn = labs(a);
        }
    }
    lseek(fd, mn, SEEK_SET);
    write(fd, zero, 1);
    close(fd);
    return 0;
}