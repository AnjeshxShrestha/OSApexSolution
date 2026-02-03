#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_PROCESSES 50
#define MAX_NAME_LEN 100

// THEORETICAL WINDOWS OS CONSTANTS (from research)
#define WINDOWS_THEORY_MIN_CS 0.0025    
#define WINDOWS_THEORY_MAX_CS 0.008     
#define WINDOWS_THEORY_CACHE 0.0015     
#define WINDOWS_THEORY_TLB 0.002        
#define WINDOWS_THEORY_SCHED 0.0012     
#define WINDOWS_THEORY_DPC 0.001        

// System profile
typedef struct {
    int cpu_cores;
    long total_memory_mb;
    char cpu_model[256];
    char windows_version[128];
} SystemProfile;

typedef struct {
    char id[10];
    char meaning[MAX_NAME_LEN];
    int arrival;
    int burst;
    int priority;
    int remaining_burst;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int start_time;
    int started;
} Process;

typedef struct {
    char pid[10];
    int start;
    int end;
} GanttEntry;

SystemProfile sys_profile;

// Function prototypes
void detect_hardware_info();
void display_system_info();
void compare_with_theory();
void display_menu();
void fcfs(Process processes[], int n);
void sjf(Process processes[], int n);
void round_robin(Process processes[], int n);
void priority_scheduling(Process processes[], int n);
void priority_round_robin(Process processes[], int n);
void display_gantt_chart(GanttEntry gantt[], int gantt_size);
void display_metrics(Process processes[], int n, int context_switches, double total_time_ms, double overhead_ms);

// Get current time in milliseconds
double get_time_ms() {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (counter.QuadPart * 1000.0) / frequency.QuadPart;
}

// Simulate process execution (1 time unit = 100ms)
void simulate_process_execution(int time_units) {
    Sleep(time_units * 100); // 100ms per time unit
}

// Simulate context switch overhead (Windows: 2.5-8 microseconds)
void simulate_context_switch() {
    // Sleep for minimum 1ms (Windows Sleep minimum)
    // To simulate microseconds, we'll just account in measurements
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    // Busy wait for ~5 microseconds (0.005 ms)
    do {
        QueryPerformanceCounter(&end);
    } while ((end.QuadPart - start.QuadPart) * 1000000.0 / frequency.QuadPart < 5);
}

// Simulate cache miss
void simulate_cache_miss() {
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    // Busy wait for ~2 microseconds
    do {
        QueryPerformanceCounter(&end);
    } while ((end.QuadPart - start.QuadPart) * 1000000.0 / frequency.QuadPart < 2);
}

// Simulate scheduler overhead
void simulate_scheduler() {
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    // Busy wait for ~1 microsecond
    do {
        QueryPerformanceCounter(&end);
    } while ((end.QuadPart - start.QuadPart) * 1000000.0 / frequency.QuadPart < 1);
}

int main() {
    Process processes[MAX_PROCESSES];
    int n = 0;
    int choice;
    
    srand((unsigned int)time(NULL));
    
    printf("================================================================\n");
    printf("   REAL-WORLD CPU SCHEDULING SIMULATOR\n");
    printf("   Platform: WINDOWS\n");
    printf("   Mode: ACTUAL TIMING with Sleep\n");
    printf("   Units: MILLISECONDS (ms)\n");
    printf("   Note: 1 time unit = 100ms actual execution\n");
    printf("================================================================\n\n");
    
    detect_hardware_info();
    display_system_info();
    compare_with_theory();
    
    // Load sample data
    printf("\nLoading sample processes...\n");
    printf("(Each process burst time unit will execute for 100ms real time)\n\n");
    n = 4;
    strcpy(processes[0].id, "P1");
    strcpy(processes[0].meaning, "Transaction Validation");
    processes[0].arrival = 0;
    processes[0].burst = 3;
    processes[0].priority = 1;
    
    strcpy(processes[1].id, "P2");
    strcpy(processes[1].meaning, "Rule/Fraud Check");
    processes[1].arrival = 1;
    processes[1].burst = 4;
    processes[1].priority = 2;
    
    strcpy(processes[2].id, "P3");
    strcpy(processes[2].meaning, "Analytics Batch Job");
    processes[2].arrival = 2;
    processes[2].burst = 10;
    processes[2].priority = 5;
    
    strcpy(processes[3].id, "P4");
    strcpy(processes[3].meaning, "Monitoring Agent");
    processes[3].arrival = 0;
    processes[3].burst = 2;
    processes[3].priority = 3;
    
    while (1) {
        display_menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        if (choice == 0) {
            printf("\nExiting program. Goodbye!\n");
            break;
        }
        
        if (choice == 6) {
            display_system_info();
            compare_with_theory();
            printf("\nPress Enter to continue...");
            while(getchar() != '\n');
            getchar();
            continue;
        }
        
        switch (choice) {
            case 1:
                fcfs(processes, n);
                break;
            case 2:
                sjf(processes, n);
                break;
            case 3:
                round_robin(processes, n);
                break;
            case 4:
                priority_scheduling(processes, n);
                break;
            case 5:
                priority_round_robin(processes, n);
                break;
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
        
        printf("\nPress Enter to continue...");
        while(getchar() != '\n');
        getchar();
        system("cls");
    }
    
    return 0;
}

void detect_hardware_info() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    sys_profile.cpu_cores = si.dwNumberOfProcessors;
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    sys_profile.total_memory_mb = (long)(memInfo.ullTotalPhys / (1024 * 1024));
    
    // Get CPU info from registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        DWORD bufferSize = sizeof(sys_profile.cpu_model);
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, 
                        (LPBYTE)sys_profile.cpu_model, &bufferSize);
        
        RegCloseKey(hKey);
    }
    
    // Get Windows version
    sprintf(sys_profile.windows_version, "Windows (Build %lu)", GetVersion());
}

void display_system_info() {
    printf("================================================================\n");
    printf("SYSTEM INFORMATION\n");
    printf("================================================================\n");
    if (strlen(sys_profile.windows_version) > 0) {
        printf("OS Version:             %s\n", sys_profile.windows_version);
    }
    if (strlen(sys_profile.cpu_model) > 0) {
        printf("CPU Model:              %s\n", sys_profile.cpu_model);
    }
    printf("CPU Cores:              %d\n", sys_profile.cpu_cores);
    printf("Total Memory:           %ld MB\n", sys_profile.total_memory_mb);
    printf("================================================================\n");
}

void compare_with_theory() {
    printf("\n================================================================\n");
    printf("THEORETICAL WINDOWS OS CONSTANTS (from research)\n");
    printf("================================================================\n");
    printf("Context Switch (min):   %.6f ms (2.5 microseconds)\n", WINDOWS_THEORY_MIN_CS);
    printf("Context Switch (max):   %.6f ms (8.0 microseconds)\n", WINDOWS_THEORY_MAX_CS);
    printf("Cache Miss Penalty:     %.6f ms (1.5 microseconds)\n", WINDOWS_THEORY_CACHE);
    printf("TLB Flush Penalty:      %.6f ms (2.0 microseconds)\n", WINDOWS_THEORY_TLB);
    printf("Scheduler Overhead:     %.6f ms (1.2 microseconds)\n", WINDOWS_THEORY_SCHED);
    printf("DPC Overhead:           %.6f ms (1.0 microseconds)\n", WINDOWS_THEORY_DPC);
    printf("\n");
    printf("SIMULATION PARAMETERS:\n");
    printf("Context Switch:         ~0.005 ms (5 microseconds)\n");
    printf("Cache Miss:             ~0.002 ms (2 microseconds)\n");
    printf("Scheduler Call:         ~0.001 ms (1 microsecond)\n");
    printf("Process Execution:      100 ms per time unit\n");
    printf("================================================================\n");
}

void display_menu() {
    printf("\n================================================================\n");
    printf("MENU - Real-World Windows Simulation\n");
    printf("================================================================\n");
    printf("1. First Come First Serve (FCFS)\n");
    printf("2. Shortest Job First (SJF)\n");
    printf("3. Round Robin (RR) [q=2]\n");
    printf("4. Priority Scheduling\n");
    printf("5. Priority Round Robin (PRR) [q=2]\n");
    printf("6. Display System Info & Constants\n");
    printf("0. Exit\n");
    printf("================================================================\n");
}

void fcfs(Process processes[], int n) {
    Process temp[MAX_PROCESSES];
    GanttEntry gantt[MAX_PROCESSES];
    int gantt_size = 0;
    
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
    }
    
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp[j].arrival > temp[j + 1].arrival) {
                Process t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }
    
    int time = 0;
    int context_switches = 0;
    
    printf("\n================================================================\n");
    printf("FCFS SCHEDULING - REAL-TIME EXECUTION\n");
    printf("================================================================\n");
    printf("Starting execution (this will take actual time)...\n\n");
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrival) {
            time = temp[i].arrival;
        }
        
        if (i > 0) {
            simulate_context_switch();
            simulate_cache_miss();
            simulate_scheduler();
            context_switches++;
        }
        
        printf("Executing %s (%s) for %d time units...\n", 
               temp[i].id, temp[i].meaning, temp[i].burst);
        
        strcpy(gantt[gantt_size].pid, temp[i].id);
        gantt[gantt_size].start = time;
        gantt[gantt_size].end = time + temp[i].burst;
        gantt_size++;
        
        temp[i].start_time = time;
        
        simulate_process_execution(temp[i].burst);
        
        temp[i].completion_time = time + temp[i].burst;
        temp[i].turnaround_time = temp[i].completion_time - temp[i].arrival;
        temp[i].waiting_time = temp[i].turnaround_time - temp[i].burst;
        
        time += temp[i].burst;
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double overhead = total_time - (temp[n-1].completion_time * 100.0);
    
    printf("\n================================================================\n");
    printf("FCFS RESULTS\n");
    printf("================================================================\n");
    
    display_gantt_chart(gantt, gantt_size);
    display_metrics(temp, n, context_switches, total_time, overhead);
}

void sjf(Process processes[], int n) {
    Process temp[MAX_PROCESSES];
    GanttEntry gantt[MAX_PROCESSES * 10];
    int gantt_size = 0;
    
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_burst = temp[i].burst;
        temp[i].started = 0;
    }
    
    int time = 0;
    int completed = 0;
    int context_switches = 0;
    int last_process = -1;
    
    printf("\n================================================================\n");
    printf("SJF SCHEDULING - REAL-TIME EXECUTION\n");
    printf("================================================================\n");
    printf("Starting execution (this will take actual time)...\n\n");
    
    double start_time = get_time_ms();
    
    while (completed < n) {
        int min_burst = 9999;
        int min_index = -1;
        
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= time && temp[i].remaining_burst > 0) {
                if (temp[i].remaining_burst < min_burst) {
                    min_burst = temp[i].remaining_burst;
                    min_index = i;
                }
            }
        }
        
        if (min_index == -1) {
            time++;
            continue;
        }
        
        if (last_process != -1 && last_process != min_index) {
            simulate_context_switch();
            simulate_cache_miss();
            simulate_scheduler();
            context_switches++;
        }
        
        if (temp[min_index].started == 0) {
            temp[min_index].start_time = time;
            temp[min_index].started = 1;
            printf("Executing %s (%s) for %d time units...\n", 
                   temp[min_index].id, temp[min_index].meaning, temp[min_index].burst);
        }
        
        strcpy(gantt[gantt_size].pid, temp[min_index].id);
        gantt[gantt_size].start = time;
        gantt[gantt_size].end = time + temp[min_index].remaining_burst;
        gantt_size++;
        
        simulate_process_execution(temp[min_index].remaining_burst);
        
        time += temp[min_index].remaining_burst;
        temp[min_index].remaining_burst = 0;
        temp[min_index].completion_time = time;
        temp[min_index].turnaround_time = temp[min_index].completion_time - temp[min_index].arrival;
        temp[min_index].waiting_time = temp[min_index].turnaround_time - temp[min_index].burst;
        
        completed++;
        last_process = min_index;
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double overhead = total_time - (time * 100.0);
    
    printf("\n================================================================\n");
    printf("SJF RESULTS\n");
    printf("================================================================\n");
    
    display_gantt_chart(gantt, gantt_size);
    display_metrics(temp, n, context_switches, total_time, overhead);
}

void round_robin(Process processes[], int n) {
    Process temp[MAX_PROCESSES];
    GanttEntry gantt[MAX_PROCESSES * 20];
    int gantt_size = 0;
    int quantum = 2;
    
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_burst = temp[i].burst;
        temp[i].started = 0;
    }
    
    int time = 0;
    int context_switches = 0;
    int last_process = -1;
    int completed = 0;
    
    printf("\n================================================================\n");
    printf("ROUND ROBIN SCHEDULING - REAL-TIME EXECUTION (Quantum=2)\n");
    printf("================================================================\n");
    printf("Starting execution (this will take actual time)...\n\n");
    
    double start_time = get_time_ms();
    
    while (completed < n) {
        int executed_something = 0;
        
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= time && temp[i].remaining_burst > 0) {
                executed_something = 1;
                
                if (last_process != -1 && last_process != i) {
                    simulate_context_switch();
                    simulate_cache_miss();
                    simulate_scheduler();
                    context_switches++;
                }
                
                if (temp[i].started == 0) {
                    temp[i].start_time = time;
                    temp[i].started = 1;
                }
                
                int exec_time = (temp[i].remaining_burst > quantum) ? quantum : temp[i].remaining_burst;
                
                printf("Executing %s (%s) for %d time units...\n", 
                       temp[i].id, temp[i].meaning, exec_time);
                
                strcpy(gantt[gantt_size].pid, temp[i].id);
                gantt[gantt_size].start = time;
                gantt[gantt_size].end = time + exec_time;
                gantt_size++;
                
                simulate_process_execution(exec_time);
                
                temp[i].remaining_burst -= exec_time;
                time += exec_time;
                
                if (temp[i].remaining_burst == 0) {
                    temp[i].completion_time = time;
                    temp[i].turnaround_time = temp[i].completion_time - temp[i].arrival;
                    temp[i].waiting_time = temp[i].turnaround_time - temp[i].burst;
                    completed++;
                }
                
                last_process = i;
            }
        }
        
        if (!executed_something) {
            time++;
        }
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double overhead = total_time - (time * 100.0);
    
    printf("\n================================================================\n");
    printf("ROUND ROBIN RESULTS\n");
    printf("================================================================\n");
    
    display_gantt_chart(gantt, gantt_size);
    display_metrics(temp, n, context_switches, total_time, overhead);
}

void priority_scheduling(Process processes[], int n) {
    Process temp[MAX_PROCESSES];
    GanttEntry gantt[MAX_PROCESSES * 10];
    int gantt_size = 0;
    
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_burst = temp[i].burst;
        temp[i].started = 0;
    }
    
    int time = 0;
    int completed = 0;
    int context_switches = 0;
    int last_process = -1;
    
    printf("\n================================================================\n");
    printf("PRIORITY SCHEDULING - REAL-TIME EXECUTION\n");
    printf("================================================================\n");
    printf("Starting execution (this will take actual time)...\n\n");
    
    double start_time = get_time_ms();
    
    while (completed < n) {
        int highest_priority = 9999;
        int min_index = -1;
        
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= time && temp[i].remaining_burst > 0) {
                if (temp[i].priority < highest_priority) {
                    highest_priority = temp[i].priority;
                    min_index = i;
                }
            }
        }
        
        if (min_index == -1) {
            time++;
            continue;
        }
        
        if (last_process != -1 && last_process != min_index) {
            simulate_context_switch();
            simulate_cache_miss();
            simulate_scheduler();
            context_switches++;
        }
        
        if (temp[min_index].started == 0) {
            temp[min_index].start_time = time;
            temp[min_index].started = 1;
            printf("Executing %s (%s) [Priority=%d] for %d time units...\n", 
                   temp[min_index].id, temp[min_index].meaning, 
                   temp[min_index].priority, temp[min_index].burst);
        }
        
        strcpy(gantt[gantt_size].pid, temp[min_index].id);
        gantt[gantt_size].start = time;
        gantt[gantt_size].end = time + temp[min_index].remaining_burst;
        gantt_size++;
        
        simulate_process_execution(temp[min_index].remaining_burst);
        
        time += temp[min_index].remaining_burst;
        temp[min_index].remaining_burst = 0;
        temp[min_index].completion_time = time;
        temp[min_index].turnaround_time = temp[min_index].completion_time - temp[min_index].arrival;
        temp[min_index].waiting_time = temp[min_index].turnaround_time - temp[min_index].burst;
        
        completed++;
        last_process = min_index;
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double overhead = total_time - (time * 100.0);
    
    printf("\n================================================================\n");
    printf("PRIORITY SCHEDULING RESULTS\n");
    printf("================================================================\n");
    
    display_gantt_chart(gantt, gantt_size);
    display_metrics(temp, n, context_switches, total_time, overhead);
}

void priority_round_robin(Process processes[], int n) {
    Process temp[MAX_PROCESSES];
    GanttEntry gantt[MAX_PROCESSES * 20];
    int gantt_size = 0;
    int quantum = 2;
    
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_burst = temp[i].burst;
        temp[i].started = 0;
    }
    
    int time = 0;
    int context_switches = 0;
    int last_process = -1;
    int completed = 0;
    
    printf("\n================================================================\n");
    printf("PRIORITY ROUND ROBIN SCHEDULING - REAL-TIME EXECUTION (Quantum=2)\n");
    printf("================================================================\n");
    printf("Starting execution (this will take actual time)...\n\n");
    
    double start_time = get_time_ms();
    
    while (completed < n) {
        int highest_priority = 9999;
        int executed_something = 0;
        
        // Find highest priority among available processes
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= time && temp[i].remaining_burst > 0) {
                if (temp[i].priority < highest_priority) {
                    highest_priority = temp[i].priority;
                }
            }
        }
        
        // Execute all processes with highest priority in round-robin
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival <= time && temp[i].remaining_burst > 0 && 
                temp[i].priority == highest_priority) {
                executed_something = 1;
                
                if (last_process != -1 && last_process != i) {
                    simulate_context_switch();
                    simulate_cache_miss();
                    simulate_scheduler();
                    context_switches++;
                }
                
                if (temp[i].started == 0) {
                    temp[i].start_time = time;
                    temp[i].started = 1;
                }
                
                int exec_time = (temp[i].remaining_burst > quantum) ? quantum : temp[i].remaining_burst;
                
                printf("Executing %s (%s) [Priority=%d] for %d time units...\n", 
                       temp[i].id, temp[i].meaning, temp[i].priority, exec_time);
                
                strcpy(gantt[gantt_size].pid, temp[i].id);
                gantt[gantt_size].start = time;
                gantt[gantt_size].end = time + exec_time;
                gantt_size++;
                
                simulate_process_execution(exec_time);
                
                temp[i].remaining_burst -= exec_time;
                time += exec_time;
                
                if (temp[i].remaining_burst == 0) {
                    temp[i].completion_time = time;
                    temp[i].turnaround_time = temp[i].completion_time - temp[i].arrival;
                    temp[i].waiting_time = temp[i].turnaround_time - temp[i].burst;
                    completed++;
                }
                
                last_process = i;
            }
        }
        
        if (!executed_something) {
            time++;
        }
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double overhead = total_time - (time * 100.0);
    
    printf("\n================================================================\n");
    printf("PRIORITY ROUND ROBIN RESULTS\n");
    printf("================================================================\n");
    
    display_gantt_chart(gantt, gantt_size);
    display_metrics(temp, n, context_switches, total_time, overhead);
}

void display_gantt_chart(GanttEntry gantt[], int gantt_size) {
    printf("\nGANTT CHART:\n");
    printf("----------------------------------------------------------------\n");
    
    for (int i = 0; i < gantt_size; i++) {
        printf("| %s ", gantt[i].pid);
    }
    printf("|\n");
    
    printf("%d", gantt[0].start);
    for (int i = 0; i < gantt_size; i++) {
        printf("    %d", gantt[i].end);
    }
    printf("\n");
}

void display_metrics(Process processes[], int n, int context_switches, double total_time_ms, double overhead_ms) {
    printf("\nPROCESS DETAILS:\n");
    printf("----------------------------------------------------------------\n");
    printf("%-6s %-30s %-4s %-4s %-4s %-4s %-4s\n", "PID", "Meaning", "AT", "BT", "CT", "TAT", "WT");
    printf("----------------------------------------------------------------\n");
    
    float total_tat = 0, total_wt = 0;
    
    for (int i = 0; i < n; i++) {
        printf("%-6s %-30s %-4d %-4d %-4d %-4d %-4d\n",
               processes[i].id,
               processes[i].meaning,
               processes[i].arrival,
               processes[i].burst,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);
        
        total_tat += processes[i].turnaround_time;
        total_wt += processes[i].waiting_time;
    }
    
    printf("\n================================================================\n");
    printf("PERFORMANCE METRICS:\n");
    printf("================================================================\n");
    printf("Average Turnaround Time:    %.2f time units\n", total_tat / n);
    printf("Average Waiting Time:       %.2f time units\n", total_wt / n);
    printf("Context Switches:           %d\n", context_switches);
    printf("Throughput:                 %.3f processes/time unit\n", 
           (float)n / processes[n-1].completion_time);
    printf("\n");
    printf("REAL MEASURED TIMING:\n");
    printf("Total Execution Time:       %.3f ms\n", total_time_ms);
    printf("Expected (100ms/unit):      %.3f ms\n", processes[n-1].completion_time * 100.0);
    printf("System Overhead:            %.3f ms\n", overhead_ms);
    printf("Overhead per context switch: %.6f ms\n", 
           context_switches > 0 ? overhead_ms / context_switches : 0);
    printf("\n");
    printf("NOTE: Overhead includes context switching, cache misses,\n");
    printf("      scheduler decisions, DPC/APC interrupts, and measurement error.\n");
    printf("================================================================\n");
}
