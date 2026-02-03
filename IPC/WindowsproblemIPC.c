#include <windows.h>
#include <stdio.h>

int main() {
    printf("P1: Starting program...\n");
    fflush(stdout);
    
    HANDLE hPipe = CreateNamedPipe(
        TEXT("\\\\.\\pipe\\ApexBatchPipe"),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,  // Non-blocking
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
        1, 1024, 1024, 0, NULL);
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("P1: Failed to create pipe\n");
        return 1;
    }
    
    printf("P1: Pipe created successfully\n");
    fflush(stdout);
    
    // Try to connect (won't block now)
    ConnectNamedPipe(hPipe, NULL);
    
    char buffer[1024];
    DWORD written;
    
    // PROBLEM: No mutex → writing without synchronization
    printf("P1: PROBLEM - writing WITHOUT waiting for anyone!\n");
    printf("P1: No mutex protection, no coordination!\n");
    fflush(stdout);
    
    BOOL result = WriteFile(hPipe, "Batch from P1", 13, &written, NULL);
    
    if (!result) {
        printf("P1: Write FAILED - no client connected (ERROR: %lu)\n", GetLastError());
        printf("P1: This shows the problem: unsynchronized access!\n");
    } else {
        printf("P1: Write succeeded (but may be lost - no receiver!)\n");
    }
    
    fflush(stdout);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    
    printf("P1: Program completed - demonstrated lack of synchronization\n");
    return 0;
}
