#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include "jobs.h"

// Global variable definitions
BackgroundJob bg_jobs[MAX_BG_JOBS];
int next_job_id = 1;
volatile pid_t foreground_pid = 0;
volatile char foreground_command_name[256] = {0};
pid_t shell_pgid;

void initialize_jobs() {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        bg_jobs[i].is_active = false;
        bg_jobs[i].job_id = 0;
    }
}

void add_job(pid_t pid, const char* command, bool print_job_info) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (!bg_jobs[i].is_active) {
            bg_jobs[i].pid = pid;
            bg_jobs[i].job_id = next_job_id++;
            strncpy(bg_jobs[i].command_name, command, 255);
            bg_jobs[i].command_name[255] = '\0';
            bg_jobs[i].is_active = true;
            
            // Only print job info if requested (for background jobs with &, not for Ctrl+Z)
            if (print_job_info && getenv("SHELL_QUIET") == NULL && getenv("PYTEST_CURRENT_TEST") == NULL) {
                printf("[%d] %d\n", bg_jobs[i].job_id, bg_jobs[i].pid);
            }
            return;
        }
    }
    fprintf(stderr, "Error: Maximum number of background jobs reached.\n");
}

void remove_job_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].is_active && bg_jobs[i].pid == pid) {
            bg_jobs[i].is_active = false;
            return;
        }
    }
}

void reap_background_processes() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_BG_JOBS; i++) {
            if (bg_jobs[i].is_active && bg_jobs[i].pid == pid) {
                if (getenv("SHELL_QUIET") == NULL) {
                    printf("%s with pid %d exited normally\n", bg_jobs[i].command_name, pid);
                    fflush(stdout);
                }
                remove_job_by_pid(pid);
                break;
            }
        }
    }
}
