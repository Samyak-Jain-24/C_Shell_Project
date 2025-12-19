#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include "job_control.h"
#include "jobs.h"
#include "process.h"

/**
 * @brief  Finds a job in the bg_jobs list by its job ID.
 * @param  job_id The job ID (e.g., 1, 2, ...).
 * @return A pointer to the BackgroundJob struct, or NULL if not found.
 */
BackgroundJob* find_job_by_id(int job_id) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].is_active && bg_jobs[i].job_id == job_id) {
            return &bg_jobs[i];
        }
    }
    return NULL;
}

/**
 * @brief  Finds the most recently created job (highest job ID).
 * @return A pointer to the BackgroundJob struct, or NULL if no active jobs.
 */
BackgroundJob* find_most_recent_job() {
    int max_job_id = -1;
    BackgroundJob* recent_job = NULL;
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (bg_jobs[i].is_active && bg_jobs[i].job_id > max_job_id) {
            max_job_id = bg_jobs[i].job_id;
            recent_job = &bg_jobs[i];
        }
    }
    return recent_job;
}

/**
 * @brief Implements the 'bg' built-in command.
 * Resumes a stopped job in the background.
 * @param args A string containing the optional job number.
 */
void execute_bg(char* args) {
    BackgroundJob* job;
    if (args == NULL) {
        // Requirement 4 (fg/bg): Use most recent job if no number is provided.
        job = find_most_recent_job();
    } else {
        job = find_job_by_id(atoi(args));
    }

    if (job == NULL) {
        // Requirement 5 (bg): No such job.
        printf("No such job\n");
        return;
    }

    pid_t pgid = getpgid(job->pid);
    const char* state = get_process_state(job->pid);

    if (strcmp(state, "Stopped") != 0) {
        // Requirement 4 (bg): Job is already running.
        printf("Job already running\n");
        return;
    }

    // Requirement 1 (bg): Resume the job by sending SIGCONT.
    if (kill(-pgid, SIGCONT) < 0) {
        perror("bg: kill (SIGCONT)");
        return;
    }
    
    // Requirement 3 (bg): Print the resumption message.
    printf("[%d] %s &\n", job->job_id, job->command_name);
}

/**
 * @brief Implements the 'fg' built-in command.
 * Brings a background or stopped job to the foreground.
 * @param args A string containing the optional job number.
 */
void execute_fg(char* args) {
    BackgroundJob* job;
    if (args == NULL) {
        // Requirement 4 (fg): Use most recent job.
        job = find_most_recent_job();
    } else {
        job = find_job_by_id(atoi(args));
    }

    if (job == NULL) {
        // Requirement 5 (fg): No such job.
        printf("No such job\n");
        return;
    }

    pid_t pgid = getpgid(job->pid);
    const char* state = get_process_state(job->pid);

    // Requirement 6 (fg): Print the command name.
    printf("%s\n", job->command_name);

    // Requirement 1 (fg): Give terminal control to the job.
    tcsetpgrp(STDIN_FILENO, pgid);

    if (strcmp(state, "Stopped") == 0) {
        // Requirement 2 (fg): Resume if stopped.
        if (kill(-pgid, SIGCONT) < 0) {
            perror("fg: kill (SIGCONT)");
            tcsetpgrp(STDIN_FILENO, shell_pgid); // Give control back on error
            return;
        }
    }

    // The job is now in the foreground, so remove it from the background list.
    pid_t job_pid = job->pid;  // Store the actual PID before removing the job
    remove_job_by_pid(job->pid);

    // Requirement 3 (fg): Wait for the job to complete or stop again.
    // This is the same logic as waiting for any other foreground process.
    foreground_pid = job_pid;  // Use actual PID, not PGID
    strcpy((char*)foreground_command_name, job->command_name);
    int status;
    waitpid(job_pid, &status, WUNTRACED);
    
    // IMPORTANT: Take terminal control back for the shell.
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    if (WIFSTOPPED(status)) {
        // If it was stopped again, re-add it to the job list.
        add_job(pgid, (const char*)job->command_name, false); // Don't print for fg command
        for (int i=0; i < MAX_BG_JOBS; i++) {
            if (bg_jobs[i].pid == pgid) {
                 printf("\n[%d] Stopped %s\n", bg_jobs[i].job_id, bg_jobs[i].command_name);
                 break;
            }
        }
    }
    foreground_pid = 0;
}
