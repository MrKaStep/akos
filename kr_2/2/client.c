#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#define E_OK   0
#define E_KEY  1
#define E_SHM  2
#define E_SEM  3
#define E_MEM  4
#define E_CONN 5

#define MAX_CLIENTS 10

#define BUF_SIZE 4096

struct sembuf data_wait[1] = {{0, -1, 0}};
struct sembuf data_ready[1] = {{0, -1, 0}};

int sem_id;

typedef struct 
{
    char s[BUF_SIZE];
    char end;
} Mem;

volatile int waiting = 1;

void handler(int signal)
{
    waiting = 0;
}

int main(int argc, const char* argv[])
{
    key_t key;
    int mem_id;
    Mem *mem;

    int ans = 0, cur = 0;

    key = ftok("/bin/ls", 1);
    if(key == -1)
    {
        perror("IPC key fail");
        return E_KEY;
    }
    mem_id = shmget(key, sizeof(Mem), 0);
    if(mem_id == -1)
    {
        perror("Shared memory fail");
        return E_SHM;
    }
    sem_id = semget(key, 10, 0);
    if(sem_id == -1)
    {
        perror("Semaphores fail");
        return E_SEM;
    }

    mem = (Mem *)shmat(mem_id, NULL, 0);
    if (mem == (void *)-1)
    {
        perror("Out of memory");
        return E_MEM;
    }

    while(1)
    {
        int l, i;
        semop(sem_id, data_wait, 1);
        if(mem->end)
        {
            semop(sem_id, data_ready, 1);
            printf("Lines containing '#' character: %d\n", ans);
            return E_OK;
        }
        l = strlen(mem->s);
        for(i = 0; i < l; ++i)
        {
            if(mem->s[i] == '#')
            {
                cur = 1;
            }
            if(mem->s[i] == '\n')
            {
                ans += cur;
                cur = 0;
            }
        }
        semop(sem_id, data_ready, 1);
    }
    return E_OK;
}