#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "process.h"
#include "jobs.h"

const char* get_process_state(pid_t pid) {
    char proc_path[256];
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/stat", pid);

    FILE* fp = fopen(proc_path, "r");
    if (fp == NULL) {
        return "Unknown"; 
    }

    char line_buffer[1024];
    if (fgets(line_buffer, sizeof(line_buffer), fp) == NULL) {
        fclose(fp);
        return "Unknown"; 
    }
    fclose(fp);
    char* state_ptr = strrchr(line_buffer, ')');
    if (state_ptr == NULL) {
        return "Unknown"; 
    }

    char state;
    if (sscanf(state_ptr + 2, " %c", &state) != 1) {
         return "Unknown"; 
    }

    if (state == 'T') {
        return "Stopped";
    } else {
        return "Running";
    }
}

int compare_jobs(const void* a, const void* b) {
    BackgroundJob* job_a = (BackgroundJob*)a;
    BackgroundJob* job_b = (BackgroundJob*)b;
    return strcmp(job_a->command_name, job_b->command_name);
}

void execute_activities() {
    BackgroundJob active_jobs[MAX_BG_JOBS];
    int active_count = 0;

    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].is_active) {
            if (kill(bg_jobs[i].pid, 0) == 0) {
                 active_jobs[active_count++] = bg_jobs[i];
            } else {
                remove_job_by_pid(bg_jobs[i].pid);
            }
        }
    }
    
    if (active_count == 0) {
        return;
    }

    qsort(active_jobs, active_count, sizeof(BackgroundJob), compare_jobs);

    for (int i = 0; i < active_count; i++) {
        pid_t pid = active_jobs[i].pid;
        const char* command_name = active_jobs[i].command_name;
        
        const char* state = get_process_state(pid);
        
        printf("[%d] : %s - %s\n", pid, command_name, state);
    }
}
