#ifndef JOBS_H
#define JOBS_H

#include <stdbool.h>
#include <unistd.h> // Required for pid_t

// Shared MACROS and TYPE DEFINITIONS
#define MAX_BG_JOBS 64

typedef struct {
    pid_t pid;
    int job_id;
    char command_name[256];
    bool is_active;
} BackgroundJob;

// Shared global variable DECLARATIONS (using 'extern')
extern BackgroundJob bg_jobs[MAX_BG_JOBS];
extern pid_t shell_pgid; // <-- The important line
extern int next_job_id;
extern volatile pid_t foreground_pid;
extern volatile char foreground_command_name[256];

// Shared FUNCTION PROTOTYPES
void initialize_jobs();
void add_job(pid_t pid, const char* command, bool print_job_info);
void remove_job_by_pid(pid_t pid);
void reap_background_processes();

#endif // JOBS_H