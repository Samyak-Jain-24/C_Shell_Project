#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h> // EDIT: Added for 'bool' type
#include<ctype.h>
#include "parser.h"
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include "reveal.h"
#include "pipe_implement.h"
#include "InputOutputRedir.h"
#include<fcntl.h>   // EDIT: Added for open()
#include<signal.h>
#include "jobs.h"
#include "BasicSysCall.h"


extern volatile pid_t foreground_pid;
extern volatile char foreground_command_name[256];

// EDIT: We need to tell this file that a function called add_job exists.
void add_job(pid_t pid, const char* command, bool print_job_info);

void parse_command_args(char* command_segment, char* argv[]) {
    int argc = 0;
    char* token = strtok(command_segment, " \t\n");
    while (token != NULL && argc < 63) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
}

// EDIT: Function signature changed to pass along background info
pid_t execute_pipe_segment(char* command_segment, int input_fd, int output_fd, bool is_first_command, bool is_background_pipeline) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // --- Child Process ---

        // EDIT: Handle stdin redirection for the FIRST command in a background pipeline
        if (is_first_command && is_background_pipeline) {
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null != -1) {
                dup2(dev_null, STDIN_FILENO);
                close(dev_null);
            }
        }

        if (input_fd != STDIN_FILENO) {
            if (dup2(input_fd, STDIN_FILENO) == -1) {
                perror("dup2 input failed");
                close(input_fd);
                exit(EXIT_FAILURE);
            }
            close(input_fd); 
        }
        
        if (output_fd != STDOUT_FILENO) {
            if (dup2(output_fd, STDOUT_FILENO) == -1) {
                perror("dup2 output failed");
                close(output_fd);
                exit(EXIT_FAILURE);
            }
            close(output_fd); 
        }
        
        char* command = strtok(command_segment, " \t\n");
        char* args = strtok(NULL, "\n");
        
        // EDIT: The individual segments of a pipeline are never "background" on their own.
        // The entire pipeline is managed as one job. So we pass 'false'.
        // You MUST ensure input_output_redirection2 is updated to accept this boolean.
        input_output_redirection2(command, args, false);
        
        exit(EXIT_SUCCESS);

    } 
    else {
        return pid;
    }
}

void execute_piped_commands(char* command_line, bool is_background) {
    if (command_line == NULL || strlen(command_line) == 0) return;

    char original_command_line[4096];
    strcpy(original_command_line, command_line);

    char* commands[64];
    int num_commands = 0;
    
    char* token = strtok(command_line, "|");
    while (token != NULL && num_commands < 64) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    if (num_commands == 0) return;

    int i;
    int prev_pipe_read_fd = STDIN_FILENO;
    int pipe_fds[2];
    pid_t pids[64];
    pid_t pgid = 0;

    for (i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            if (pipe(pipe_fds) == -1) {
                perror("pipe");
                return;
            }
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            return;
        }

        if (pids[i] == 0) { // --- Child Process ---
            // This is the new, corrected logic for the child.
            // It does not call any other forking functions.

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (i == 0) { pgid = getpid(); }
            setpgid(0, pgid);

            if (i == 0 && is_background) {
                int dev_null = open("/dev/null", O_RDONLY);
                if (dev_null != -1) { dup2(dev_null, STDIN_FILENO); close(dev_null); }
            }

            if (i > 0) {
                dup2(prev_pipe_read_fd, STDIN_FILENO);
                close(prev_pipe_read_fd);
            }

            if (i < num_commands - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }
            
            // This child will now parse and execute itself.
            // Parse the command properly and execute it
            char* command_copy = strdup(commands[i]);
            
            // Trim whitespace
            char* start = command_copy;
            while (isspace(*start)) start++;
            char* end = start + strlen(start) - 1;
            while (end > start && isspace(*end)) *end-- = '\0';
            
            // Parse command and arguments
            char* cmd = strtok(start, " \t");
            if (cmd == NULL) {
                free(command_copy);
                exit(EXIT_FAILURE);
            }
            
            // Check if it's a builtin command
            if (strcmp(cmd, "reveal") == 0) {
                // Handle reveal builtin  
                char* args = strtok(NULL, "");
                extern void reveal(char *currpath, char *args, char *root);
                extern char* get_current_path(); // You'll need to implement this
                extern char* get_home_path();    // You'll need to implement this
                // For now, use basic paths - you might need to pass these from main
                char cwd[256], home[256];
                getcwd(cwd, sizeof(cwd));
                strcpy(home, getenv("HOME") ? getenv("HOME") : cwd);
                reveal(cwd, args, home);
            } else if (strcmp(cmd, "echo") == 0) {
                // Handle echo builtin
                char* args = strtok(NULL, "");
                if (args) {
                    printf("%s\n", args);
                } else {
                    printf("\n");
                }
            } else {
                // External command - use execvp
                char* argv[64];
                int argc = 0;
                argv[argc++] = cmd;
                
                char* arg = strtok(NULL, " \t");
                while (arg != NULL && argc < 63) {
                    argv[argc++] = arg;
                    arg = strtok(NULL, " \t");
                }
                argv[argc] = NULL;
                
                execvp(cmd, argv);
                // If execvp fails
                printf("Command not found!\n");
            }
            
            free(command_copy);
            exit(EXIT_SUCCESS);
        }

        // --- Parent Process --- (This part was mostly correct)
        if (i == 0) {
            pgid = pids[0];
        }
        setpgid(pids[i], pgid);

        if (prev_pipe_read_fd != STDIN_FILENO) {
            close(prev_pipe_read_fd);
        }
        if (i < num_commands - 1) {
            close(pipe_fds[1]);
            prev_pipe_read_fd = pipe_fds[0];
        }
    }

    // --- Parent Process (After loop) ---
    // The waiting logic for job control remains the same.
    if (!is_background) {
        foreground_pid = pgid;
        strncpy((char*)foreground_command_name, original_command_line, 255);

        for (i = 0; i < num_commands; i++) {
            int status;
            waitpid(pids[i], &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                add_job(pgid, original_command_line, false); // Don't print for stopped processes
                 for (int j=0; j < MAX_BG_JOBS; j++) {
                    if (bg_jobs[j].is_active && bg_jobs[j].pid == pgid) {
                         printf("\n[%d] Stopped %s\n", bg_jobs[j].job_id, bg_jobs[j].command_name);
                         break;
                    }
                }
                foreground_pid = 0;
                tcsetpgrp(STDIN_FILENO, shell_pgid);
                return;
            }
        }
        tcsetpgrp(STDIN_FILENO, shell_pgid);
        foreground_pid = 0;
    } else {
        add_job(pgid, original_command_line, true); // Print for background processes
    }
}
