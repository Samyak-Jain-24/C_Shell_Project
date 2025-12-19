#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <termios.h>
#include <errno.h>
#include "parser.h"
#include "history.h"
#include "jobs.h"
#include "signals.h"
#include "path_utils.h"
#include "command_processor.h"

int main(){
    // Test-compatible setup
    shell_pgid = getpid();
    
    // Setup signal handlers
    setup_signal_handlers();
    
    // Only try job control in truly interactive mode
    if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO) && getenv("TERM") != NULL) {
        setpgid(shell_pgid, shell_pgid);
        tcsetpgrp(STDIN_FILENO, shell_pgid);
    }
    
    char* username = getenv("USER");
    char hostname[256];
    gethostname(hostname,sizeof(hostname));
    char cwd[256]; // This is the home directory (initial working directory)
    getcwd(cwd,sizeof(cwd));
    char currpath[256]; // This will be our dynamic current path
    strcpy(currpath,cwd);
    initialize_jobs();
    init_history();

    while(1){
        reap_background_processes();
        // Print prompt
        printf("<%s@%s:",username,hostname);
        char ans[256];
        GetCorrectPath(currpath,cwd,ans);
        printf("%s> ",ans);
        fflush(stdout);

        char input[4096]; // Increased size for longer command sequences
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Check if fgets failed due to an interrupted system call (EINTR)
            // This happens when Ctrl+C is pressed and interrupts fgets
            if (errno == EINTR) {
                // Clear the error and continue the loop to redisplay prompt
                clearerr(stdin);
                errno = 0;
                continue;
            }
            
            // Check if it's a real EOF (feof returns non-zero for EOF)
            if (feof(stdin)) {
                // Handle Ctrl+D (EOF) to exit the shell gracefully
                // This block is executed on Ctrl+D (EOF)

                // 1. First, kill all active background jobs
                for (int i = 0; i < MAX_BG_JOBS; i++) {
                    if (bg_jobs[i].is_active) {
                        kill(bg_jobs[i].pid, SIGKILL);
                    }
                }

                // 2. Then, print the logout message
                printf("logout\n");
                
                // 3. Force the message to the screen
                fflush(stdout);
                
                // 4. Finally, break from the loop to exit the shell
                break;
            }
            
            // Some other error occurred, clear it and continue
            clearerr(stdin);
            continue;
        }

        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        // Skip empty input
        if (strlen(input) == 0) {
            continue;
        }

        // Add to history before parsing, unless it's the log command
        char history_check_copy[4096];
        strcpy(history_check_copy, input);
        char* first_word_for_history = strtok(history_check_copy, " \t\n");
        if (first_word_for_history != NULL && strcmp(first_word_for_history, "log") != 0 && strcmp(first_word_for_history, "LOG") != 0) {
            add_to_history(input);
        }

        // Check syntax before processing
        if (!is_valid_syntax(input)) {
            printf("Invalid Syntax!\n");
            continue;
        }

        // Process semicolon and ampersand separated commands
        // First, let's handle the case where & separates commands (not just background)
        char input_copy[4096];
        strcpy(input_copy, input);
        
        // Split by both ; and & but track which separator was used
        char* commands[64];
        bool is_background[64];
        int cmd_count = 0;
        
        char* current_pos = input_copy;
        char* cmd_start = current_pos;
        
        while (*current_pos != '\0' && cmd_count < 64) {
            if (*current_pos == ';' || *current_pos == '&') {
                char separator = *current_pos;
                *current_pos = '\0'; // Null terminate the command
                
                // Store the command and whether it should be background
                commands[cmd_count] = strdup(cmd_start);
                is_background[cmd_count] = (separator == '&');
                cmd_count++;
                
                // Move to next command
                current_pos++;
                while (*current_pos == ' ' || *current_pos == '\t') current_pos++; // Skip whitespace
                cmd_start = current_pos;
            } else {
                current_pos++;
            }
        }
        
        // Handle the last command (no separator after it)
        if (cmd_count < 64 && strlen(cmd_start) > 0) {
            commands[cmd_count] = strdup(cmd_start);
            // Check if the last command ends with &
            char* end = cmd_start + strlen(cmd_start) - 1;
            while (end > cmd_start && (*end == ' ' || *end == '\t')) end--;
            if (*end == '&') {
                *end = '\0';
                is_background[cmd_count] = true;
            } else {
                is_background[cmd_count] = false;
            }
            cmd_count++;
        }
        
        // Process each command
        for (int i = 0; i < cmd_count; i++) {
            // Reap any background processes that completed before executing this command
            reap_background_processes();
            
            // If this command should be background, append & to it for process_single_command
            if (is_background[i]) {
                char bg_cmd[4096];
                snprintf(bg_cmd, sizeof(bg_cmd), "%s &", commands[i]);
                process_single_command(bg_cmd, currpath, cwd);
            } else {
                process_single_command(commands[i], currpath, cwd);
            }
            free(commands[i]);
            
            // Reap any background processes that completed during command execution
            reap_background_processes();
        }
    }
    return 0;   
}
