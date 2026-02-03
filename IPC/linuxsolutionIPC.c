#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define SIZE 1024

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main() {
    int shmid = shmget(SHM_KEY, SIZE, 0666 | IPC_CREAT);
    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);

    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);  // binary semaphore = 1

    char *shm = (char *)shmat(shmid, NULL, 0);

    // ← show that we are waiting for the lock
    printf("P1: trying to acquire semaphore...\n");
    struct sembuf lock = {0, -1, 0};   // P (wait)
    semop(semid, &lock, 1);
    printf("P1: got semaphore, now writing\n");

    sprintf(shm, "Batch from P1 at %d", getpid());
    printf("P1 wrote: %s\n", shm);

    sleep(1);  // simulate work

    // ← show that we release the lock
    printf("P1: releasing semaphore\n");
    struct sembuf unlock = {0, 1, 0};  // V (signal)
    semop(semid, &unlock, 1);

    shmdt(shm);
    return 0;
}
