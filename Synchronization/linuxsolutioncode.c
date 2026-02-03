#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>

void* transaction_validation_fixed(void* arg) {
    printf("✅ P1: Opening both files first\n");
    int fdA = open("data.lock", O_RDWR|O_CREAT, 0644);
    int fdB = open("rules.lock", O_RDWR|O_CREAT, 0644);
    
    printf("✅ P1: Lock A → then B (ORDERED)\n");
    flock(fdA, LOCK_EX);  // 1. ALWAYS A FIRST
    printf("   P1: Resource A locked ✓\n");
    sleep(1);
    
    flock(fdB, LOCK_EX);  // 2. THEN B
    printf("✅ P1: Both resources → Validating transaction ✓\n");
    sleep(1);
    
    flock(fdB, LOCK_UN);
    flock(fdA, LOCK_UN);
    close(fdA); close(fdB);
    printf("✅ P1: Released both ✓\n");
    return NULL;
}

void* fraud_check_fixed(void* arg) {
    printf("✅ P2: Opening both files first\n");
    int fdA = open("data.lock", O_RDWR|O_CREAT, 0644);
    int fdB = open("rules.lock", O_RDWR|O_CREAT, 0644);
    
    printf("✅ P2: Lock A → then B (ORDERED)\n");  
    flock(fdA, LOCK_EX);  // 1. ALWAYS A FIRST (WAITS if P1 holds)
    printf("   P2: Resource A locked ✓\n");
    sleep(1);
    
    flock(fdB, LOCK_EX);  // 2. THEN B
    printf("✅ P2: Both resources → Fraud check complete ✓\n");
    sleep(1);
    
    flock(fdB, LOCK_UN);
    flock(fdA, LOCK_UN);
    close(fdA); close(fdB);
    printf("✅ P2: Released both ✓\n");
    return NULL;
}

int main() {
    pthread_t p1, p2;
    printf("=== APEXONE DEADLOCK FIXED ===\n");
    pthread_create(&p1, NULL, transaction_validation_fixed, NULL);
    pthread_create(&p2, NULL, fraud_check_fixed, NULL);
    pthread_join(p1, NULL);  // COMPLETES NORMALLY
    pthread_join(p2, NULL);
    printf("🎉 SUCCESS: No deadlock!\n");
    return 0;
}
