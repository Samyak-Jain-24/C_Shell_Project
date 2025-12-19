#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "signals.h"
#include "jobs.h"

void handle_sigint(int sig) {
    (void)sig; 
    if (foreground_pid != 0) {
        // There's a foreground process running, kill it
        kill(-foreground_pid, SIGINT);
        int status;
        waitpid(foreground_pid, &status, WNOHANG);
        foreground_pid = 0;
        
        // Print newline after killing foreground process
        if (isatty(STDIN_FILENO)) {
            write(STDOUT_FILENO, "\n", 1);
        } else {
            printf("\n");
            fflush(stdout);
        }
    } else {
        // No foreground process running - just print a newline for clean prompt
        // and let the main loop continue
        if (isatty(STDIN_FILENO)) {
            write(STDOUT_FILENO, "\n", 1);
        } else {
            printf("\n");
            fflush(stdout);
        }
    }
}

void handle_sigtstp(int sig) {
    (void)sig; 
    if (foreground_pid != 0) {
        kill(-foreground_pid, SIGTSTP);
        add_job(foreground_pid, (const char*)foreground_command_name, false); // Don't print job info for Ctrl+Z
        for (int i=0; i < MAX_BG_JOBS; i++) {
            if (bg_jobs[i].is_active && bg_jobs[i].pid == foreground_pid) {
                 printf("[%d] Stopped %s\n", bg_jobs[i].job_id, bg_jobs[i].command_name);
                 fflush(stdout);
                 break;
            }
        }
        foreground_pid = 0;
    }
    // Ensure prompt returns properly
    if (isatty(STDIN_FILENO)) {
        write(STDOUT_FILENO, "\n", 1);
    } else {
        printf("\n");
        fflush(stdout);
    }
}

void setup_signal_handlers() {
    struct sigaction sa_int, sa_tstp, sa_ignore;
    
    // Setup SIGINT handler (Ctrl+C)
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0; // Don't use SA_RESTART for SIGINT so fgets() gets interrupted
    sigaction(SIGINT, &sa_int, NULL);
    
    // Setup SIGTSTP handler (Ctrl+Z)
    sa_tstp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART; // Automatically restart interrupted system calls
    sigaction(SIGTSTP, &sa_tstp, NULL);
    
    // Ignore other signals
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;
    sigaction(SIGQUIT, &sa_ignore, NULL);
    sigaction(SIGTTOU, &sa_ignore, NULL);
    sigaction(SIGTTIN, &sa_ignore, NULL);
}
