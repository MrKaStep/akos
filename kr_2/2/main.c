#define _XOPEN_SOURCE 1000

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#define E_OK   0
#define E_KEY  1
#define E_SHM  2
#define E_SEM  3
#define E_MEM  4
#define E_ARG  5
#define E_OPEN 6

#define MAX_CLIENTS 10

#define BUF_SIZE 4096

int mem_id;
int sem_id;

struct sembuf data_ready[1] = {{0, 2, 0}};
struct sembuf server_wait[1] = {{0, 0, 0}};

typedef struct 
{
    char s[BUF_SIZE];
    char end;
} Mem;

void ctrl_c(int signal) 
{
    write(1, "Bye!\n", 5);
    shmctl(mem_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    exit(E_OK);
}

void consume_line(FILE* f) {
    char tmp[BUF_SIZE];
    while(1)
    {
        int l, i;
        if(fgets(tmp, BUF_SIZE - 1, f) == NULL)
        {
            return;
        }
        l = strlen(tmp);
        for(i = 0; i < l; ++i)
        {
            if(tmp[i] == '\n')
            {
                return;
            }
        }
    }
}

int main(int argc, const char* argv[])
{
    key_t key;
    FILE *f;
    Mem *mem;
    int i, cur = 1;

    if(argc != 2)
    {
        fprintf(stderr, "\tInvalid number of arguments\n");
        return E_ARG;
    }
    f = fopen(argv[1], "r");
    if(f == NULL)
    {
        perror("File open failed");
        return E_OPEN;   
    }
    key = ftok("/bin/ls", 1);
    if(key == -1)
    {
        perror("IPC key fail");
        return E_KEY;
    }
    mem_id = shmget(key, sizeof(Mem), IPC_CREAT | IPC_EXCL | 0660);
    if(mem_id == -1)
    {
        perror("Shared memory fail");
        return E_SHM;
    }
    sem_id = semget(key, 10, IPC_CREAT | IPC_EXCL | 0660);
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

    signal(SIGINT, ctrl_c);
    mem->end = 0;
    while(fgets(mem->s, BUF_SIZE - 1, f))
    {
        int l, i;
        l = strlen(mem->s);
        for(i = 0; i < l; ++i)
        {
            if(mem->s[i] == '\n')
            {
                ++cur;
            }
        }
        semop(sem_id, data_ready, 1);
        semop(sem_id, server_wait, 1);
        if(cur % 2 == 0)
        {
            consume_line(f);
            ++cur;
        }
    }
    mem->end = 1;
    semop(sem_id, data_ready, 1);
    semop(sem_id, server_wait, 1);
    ctrl_c(0);

    return E_OK;
}