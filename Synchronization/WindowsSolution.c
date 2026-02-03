#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

DWORD WINAPI transaction_validation_fixed(LPVOID arg) {
    printf("✅ P1: Creating Windows named mutexes (Global scope)\n");
    HANDLE mutexA = CreateMutex(NULL, FALSE, "Global\\ApexOne_Data");
    HANDLE mutexB = CreateMutex(NULL, FALSE, "Global\\ApexOne_Rules");
    
    printf("✅ P1: Acquiring A → then B (ORDERED LOCKING)\n");
    WaitForSingleObject(mutexA, INFINITE);  // 1. ALWAYS RESOURCE A FIRST
    printf("   P1: Resource A locked ✓\n");
    Sleep(1000);
    
    WaitForSingleObject(mutexB, INFINITE);  // 2. THEN RESOURCE B
    printf("✅ P1: Both resources acquired → Validating transaction ✓\n");
    Sleep(1000);
    
    ReleaseMutex(mutexB);
    ReleaseMutex(mutexA);
    CloseHandle(mutexB);
    CloseHandle(mutexA);
    printf("✅ P1: All resources released ✓\n");
    return 0;
}

DWORD WINAPI fraud_check_fixed(LPVOID arg) {
    printf("✅ P2: Creating Windows named mutexes (Global scope)\n");
    HANDLE mutexA = CreateMutex(NULL, FALSE, "Global\\ApexOne_Data");
    HANDLE mutexB = CreateMutex(NULL, FALSE, "Global\\ApexOne_Rules");
    
    printf("✅ P2: Acquiring A → then B (ORDERED LOCKING)\n");
    WaitForSingleObject(mutexA, INFINITE);  // 1. ALWAYS RESOURCE A FIRST (waits for P1)
    printf("   P2: Resource A locked ✓\n");
    Sleep(1000);
    
    WaitForSingleObject(mutexB, INFINITE);  // 2. THEN RESOURCE B
    printf("✅ P2: Both resources acquired → Fraud check complete ✓\n");
    Sleep(1000);
    
    ReleaseMutex(mutexB);
    ReleaseMutex(mutexA);
    CloseHandle(mutexB);
    CloseHandle(mutexA);
    printf("✅ P2: All resources released ✓\n");
    return 0;
}

int main() {
    printf("=== APEXONE WINDOWS DEADLOCK FIXED ===\n");
    HANDLE hThread1 = CreateThread(NULL, 0, transaction_validation_fixed, NULL, 0, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, fraud_check_fixed, NULL, 0, NULL);
    WaitForSingleObject(hThread1, INFINITE);  // COMPLETES NORMALLY
    WaitForSingleObject(hThread2, INFINITE);
    CloseHandle(hThread1); CloseHandle(hThread2);
    printf("🎉 SUCCESS: Windows deadlock eliminated!\n");
    return 0;
}
