#define _XOPEN_SOURCE 1000

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

long s, ns;

void set_timer()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    s = t.tv_sec;
    ns = t.tv_nsec;
}

long get_time_in_ns()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_nsec - ns + 1000000000l * (t.tv_sec - s);
}

void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void random_shuffle(int* begin, int* end)
{
    int* i;
    for(i = begin; i != end - 1; ++i)
    {
        int offset = rand() % (end - i - 1);
        swap(i, i + offset);
    }
}

void measure_and_print(uint64_t* a, int* order, size_t l)
{
    int i;
    long t;
    uint64_t sum = 0;
    set_timer();
    for(i = 0; i < l; ++i)
    {
        sum += a[order[i]];
    }
    t = get_time_in_ns();
    printf("%ld %lu\n", t, sum);
}

void init_array(uint64_t** a, int l)
{
    int i;
    if(*a != NULL)
        free(*a);
    *a = calloc(l, sizeof(uint64_t));
    for(i = 0; i < l; ++i)
        (*a)[i] = l - i;
}

void init_order(int** order, int l, int step)
{
    int i;
    if(*order != NULL)
        free(*order);
    *order = calloc(l, sizeof(int));
    for(i = 0; i < l; ++i)
    {
        (*order)[i] = (int)(((long)i * step) % l);
    }
}

int main()
{
    int i, j, l, quant = 1 << 18, cnt = 256;
    uint64_t *a = NULL;
    int *order = NULL;
    for(i = 1; i <= cnt; ++i)
    {
        l = i * quant;
        init_array(&a, l);
        init_order(&order, l, 1);
        measure_and_print(a, order, l);
    }
    for(i = 1; i <= cnt; ++i)
    {
        l = i * quant;
        init_array(&a, l);
        init_order(&order, l, 333);
        measure_and_print(a, order, l);
    }
    for(i = 1; i <= cnt; ++i)
    {
        l = i * quant;
        init_array(&a, l);
        init_order(&order, l, 1 << 10);
        measure_and_print(a, order, l);
    }
    for(i = 1; i <= cnt; ++i)
    {
        l = i * quant;
        init_array(&a, l);
        init_order(&order, l, 1 << 15);
        measure_and_print(a, order, l);
    }
    for(i = 1; i <= cnt; ++i)
    {
        l = i * quant;
        init_array(&a, l);
        init_order(&order, l, 1);
        random_shuffle(order, order + l);
        measure_and_print(a, order, l);
    }
    free(a);
    free(order);
}