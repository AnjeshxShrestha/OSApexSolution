#include <windows.h>
#include <stdio.h>

int main() {
    printf("P1: Starting with mutex protection...\n");
    fflush(stdout);
    
    HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("ApexBatchMutex"));
    
    HANDLE hPipe = CreateNamedPipe(
        TEXT("\\\\.\\pipe\\ApexBatchPipe"),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
        1, 1024, 1024, 5000, NULL);
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("P1: Failed to create pipe\n");
        return 1;
    }
    
    printf("P1: Pipe created\n");
    fflush(stdout);
    
    // Non-blocking connect attempt
    ConnectNamedPipe(hPipe, NULL);
    
    // SOLUTION: Using mutex for synchronization
    printf("P1: trying to acquire mutex...\n");
    fflush(stdout);
    
    WaitForSingleObject(hMutex, INFINITE);
    
    printf("P1: got mutex, now writing\n");
    fflush(stdout);
    
    char buffer[1024];
    DWORD written;
    WriteFile(hPipe, "Batch from P1", 13, &written, NULL);
    
    printf("P1 sent batch safely with mutex protection.\n");
    fflush(stdout);
    
    Sleep(1000);  // simulate work
    
    printf("P1: releasing mutex\n");
    fflush(stdout);
    
    ReleaseMutex(hMutex);
    
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    CloseHandle(hMutex);
    
    printf("P1: Completed successfully with synchronization!\n");
    return 0;
}
