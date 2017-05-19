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

void print_2(unsigned long a)
{
    int i;
    for(i = long_bits - 1; i >= 0; --i)
    {
        printf("%d", (a & (1l << i)) > 0 ? 1 : 0);
    }
    puts("");
}

void move_bits(unsigned long* a, int p_left, int p_right, int move_size)
{
    unsigned long mask_left, mask_right, n_left, n_right;
    if(p_left == 1)
        mask_left = 1ul << (long_bits - 1);
    else
        mask_left = (1ul << (long_bits - p_left + 1)) - (1ul << (long_bits - p_left - move_size + 1));
    
    if(p_right == 1)
        mask_right = 1ul << (long_bits - 1);
    else
        mask_right = (1ul << (long_bits - p_right + 1)) - (1ul << (long_bits - p_right - move_size + 1));
    n_left = *a & mask_left;
    n_right = *a & mask_right;
    *a &= ~(mask_left | mask_right);
    *a |= (n_left >> (p_right - p_left));
    *a |= (n_right << (p_right - p_left));
    print_2(mask_left);
    print_2(mask_right);
    print_2(~(mask_left | mask_right));
}

int main(int argc, const char* argv[])
{
    int p_left, p_right, move_size;
    int fd;
    long a, mn = LONG_MAX;
    unsigned char zero = 0;
    char s[1024];
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
    while(fgets(s, 1023, stdin) != NULL)
    {
        a = strtol(s, NULL, 16);
        print_2(a);
        move_bits((unsigned long *)&a, p_left, p_right, move_size);
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