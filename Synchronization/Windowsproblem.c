#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

DWORD WINAPI transaction_validation(LPVOID arg) {
    printf("🚨 P1: Locked RESOURCE A (Transaction data) - Windows file lock\n");
    HANDLE hFileA = CreateFile("data.lock", GENERIC_READ|GENERIC_WRITE, 0, 
                              NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    OVERLAPPED ovA = {0};
    LockFileEx(hFileA, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ovA);  // HOLDS A
    
    Sleep(1000);  // Simulate processing
    
    printf("🚨 P1: WAITING for RESOURCE B (Fraud rules)...\n");
    HANDLE hFileB = CreateFile("rules.lock", GENERIC_READ|GENERIC_WRITE, 0, 
                              NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    OVERLAPPED ovB = {0};
    LockFileEx(hFileB, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ovB);  // DEADLOCK!
    
    UnlockFileEx(hFileB, 0, 1, 0, &ovB); CloseHandle(hFileB);
    UnlockFileEx(hFileA, 0, 1, 0, &ovA); CloseHandle(hFileA);
    return 0;
}

DWORD WINAPI fraud_check(LPVOID arg) {
    printf("🚨 P2: Locked RESOURCE B (Fraud rules) - Windows file lock\n");
    HANDLE hFileB = CreateFile("rules.lock", GENERIC_READ|GENERIC_WRITE, 0, 
                              NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    OVERLAPPED ovB = {0};
    LockFileEx(hFileB, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ovB);  // HOLDS B
    
    Sleep(1000);  // Simulate processing
    
    printf("🚨 P2: WAITING for RESOURCE A (Transaction data)...\n");
    HANDLE hFileA = CreateFile("data.lock", GENERIC_READ|GENERIC_WRITE, 0, 
                              NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    OVERLAPPED ovA = {0};
    LockFileEx(hFileA, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ovA);  // DEADLOCK!
    
    UnlockFileEx(hFileA, 0, 1, 0, &ovA); CloseHandle(hFileA);
    UnlockFileEx(hFileB, 0, 1, 0, &ovB); CloseHandle(hFileB);
    return 0;
}

int main() {
    printf("=== APEXONE WINDOWS DEADLOCK SIMULATION ===\n");
    HANDLE hThread1 = CreateThread(NULL, 0, transaction_validation, NULL, 0, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, fraud_check, NULL, 0, NULL);
    WaitForSingleObject(hThread1, INFINITE);  // HANGS FOREVER
    WaitForSingleObject(hThread2, INFINITE);
    CloseHandle(hThread1); CloseHandle(hThread2);
    return 0;
}
