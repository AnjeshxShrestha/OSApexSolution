#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define SIZE 1024

int main() {
    int shmid = shmget(SHM_KEY, SIZE, 0666 | IPC_CREAT);
    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    char *shm = (char *)shmat(shmid, NULL, 0);

    // PROBLEM: no lock → race condition
    sprintf(shm, "Batch from P1 at %d", getpid());
    printf("P1 wrote: %s\n", shm);

    // ← show that this runs without waiting
    printf("P1: writing WITHOUT waiting for anyone\n");

    sleep(1);  // make it easier to see overlap

    shmdt(shm);
    return 0;
}
