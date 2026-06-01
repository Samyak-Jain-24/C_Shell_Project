#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>      // EDIT: Added for the 'bool' type
#include<ctype.h>
#include "parser.h"
#include "BasicSysCall.h"
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>        // EDIT: Added for the open() function

/* =========================================================================
 * GLOBALS
 * =========================================================================
 * The following global definitions are used to keep track of the foreground
 * process state across different source files in the project.
 * ========================================================================= */

extern volatile pid_t foreground_pid;
extern volatile char foreground_command_name[256];

// EDIT: We need to declare add_job so this file knows it exists.
// The actual function is in your main.c file.
void add_job(pid_t pid, const char* command, bool print_job_info);

// EDIT: The function signature is changed to accept a boolean flag.
void execute_command(char* command, char* args, bool is_background) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // --- Child process ---

        // EDIT: If it's a background command, redirect its input so it doesn't
        // get stuck waiting for the keyboard.
        if (is_background) {
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null != -1) {
                dup2(dev_null, STDIN_FILENO);
                close(dev_null);
            }
        }

        // Your original argument handling is preserved.
        char* argv[] = {command, args, NULL};
        execvp(command, argv);
        
        // If execvp returns, there was an error
        printf("Command not found!\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Fork failed
        perror("fork");
    } else { // --- Parent process ---
        if (is_background) {
            // Reconstruct the full command for job tracking
            char full_command[512];
            if (args && strlen(args) > 0) {
                snprintf(full_command, sizeof(full_command), "%s %s &", command, args);
            } else {
                snprintf(full_command, sizeof(full_command), "%s &", command);
            }
            add_job(pid, full_command, true); // Print for background processes
        } else {
            // It's a foreground process, so we track its PID and name.
            foreground_pid = pid;
            strncpy((char*)foreground_command_name, command, 255); // or actual_command

            int status;
            // EDIT: Use waitpid with the WUNTRACED flag. This is the critical fix.
            waitpid(pid, &status, WUNTRACED);

            // If the process wasn't stopped (it terminated), clear the foreground tracker.
            // If it WAS stopped, the SIGTSTP handler already took care of everything.
            if (!WIFSTOPPED(status)) {
                foreground_pid = 0;
            }
        }
    }   
}
