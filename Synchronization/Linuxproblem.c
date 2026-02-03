#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>

void* transaction_validation(void* arg) {  // P1 SERVICE
    printf("🚨 P1: Locked RESOURCE A (Transaction data)\n");
    int fdA = open("data.lock", O_RDWR|O_CREAT, 0644);
    flock(fdA, LOCK_EX);  // HOLDS A
    
    sleep(1);  // Simulate processing
    
    printf("🚨 P1: WAITING for RESOURCE B (Fraud rules)...\n");
    int fdB = open("rules.lock", O_RDWR|O_CREAT, 0644);
    flock(fdB, LOCK_EX);  // DEADLOCK! P2 holds B
    // NEVER REACHES HERE
    
    flock(fdB, LOCK_UN); close(fdB);
    flock(fdA, LOCK_UN); close(fdA);
    return NULL;
}

void* fraud_check(void* arg) {  // P2 SERVICE  
    printf("🚨 P2: Locked RESOURCE B (Fraud rules)\n");
    int fdB = open("rules.lock", O_RDWR|O_CREAT, 0644);
    flock(fdB, LOCK_EX);  // HOLDS B
    
    sleep(1);  // Simulate processing
    
    printf("🚨 P2: WAITING for RESOURCE A (Transaction data)...\n");
    int fdA = open("data.lock", O_RDWR|O_CREAT, 0644);
    flock(fdA, LOCK_EX);  // DEADLOCK! P1 holds A
    // NEVER REACHES HERE
    
    flock(fdA, LOCK_UN); close(fdA);
    flock(fdB, LOCK_UN); close(fdB);
    return NULL;
}

int main() {
    pthread_t p1, p2;
    printf("=== APEXONE DEADLOCK SIMULATION ===\n");
    pthread_create(&p1, NULL, transaction_validation, NULL);
    pthread_create(&p2, NULL, fraud_check, NULL);
    pthread_join(p1, NULL);  // HANGS FOREVER
    pthread_join(p2, NULL);
    return 0;
}
