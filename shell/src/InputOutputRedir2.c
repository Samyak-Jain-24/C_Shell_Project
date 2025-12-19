#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include "InputOutputRedir.h"

extern volatile pid_t foreground_pid;
extern volatile char foreground_command_name[256];

void add_job(pid_t pid, const char* command, bool print_job_info);

void input_output_redirection2(char* command, char* currpath, char* homepath) {
    if (command == NULL) return;

    // Use the full command as passed in
    char full_command[512];
    snprintf(full_command, sizeof(full_command), "%s", command);
    
    // Check if it's a background process (ends with &)
    bool is_background = false;
    int len = strlen(full_command);
    for (int i = len - 1; i >= 0; i--) {
        if (full_command[i] == '&') {
            is_background = true;
            full_command[i] = '\0'; // Remove the &
            break;
        } else if (!isspace(full_command[i])) {
            break; // Found non-whitespace before &, so no background
        }
    }

    
    // Variables to store redirection information
    char* input_file = NULL;
    char* output_file = NULL;
    int is_append = 0;
    char* temp; // Temporary pointer for string operations
    
    // Create a copy for parsing without modifying the original
    char command_copy[512];
    strcpy(command_copy, full_command);
    
    // Find ALL input redirections and process them in order, but only the last valid one is used
    char* temp_copy = strdup(command_copy);
    char* input_redirects[10];
    int input_count = 0;
    temp = temp_copy;
    
    while ((temp = strstr(temp, "<")) != NULL && input_count < 10) {
        input_redirects[input_count++] = temp;
        temp++; // Move past the '<' to find next occurrence
    }
    
    // Process all input redirections to check for file existence errors
    for (int i = 0; i < input_count; i++) {
        char* file_start = input_redirects[i] + 1;
        while (*file_start == ' ' || *file_start == '\t') {
            file_start++;
        }
        
        if (*file_start != '\0' && *file_start != '\n') {
            // Find end of filename
            char* file_end = file_start;
            while (*file_end != ' ' && *file_end != '\t' && *file_end != '\n' && *file_end != '\0' &&
                   *file_end != '<' && *file_end != '>') {
                file_end++;
            }
            
            // Check if file exists
            char test_filename[256];
            int filename_len = file_end - file_start;
            strncpy(test_filename, file_start, filename_len);
            test_filename[filename_len] = '\0';
            
            if (access(test_filename, R_OK) != 0) {
                printf("No such file or directory\n");
                free(temp_copy);
                if (input_file) free(input_file);
                if (output_file) free(output_file);
                return;
            }
            
            // If this is the last input redirection, save it
            if (i == input_count - 1) {
                input_file = malloc(filename_len + 1);
                strcpy(input_file, test_filename);
            }
        }
    }
    
    free(temp_copy);
    
    // Find the LAST input redirection for command parsing
    char* last_input_redirect = NULL;
    char* temp2 = command_copy;
    while ((temp2 = strstr(temp2, "<")) != NULL) {
        last_input_redirect = temp2;
        temp2++; // Move past the '<' to find next occurrence
    }
    
    
    // Find ALL output redirections and process them in order
    temp_copy = strdup(command_copy);
    char* output_redirects[10];
    int output_count = 0;
    temp = temp_copy;
    
    while (*temp != '\0') {
        if (*temp == '>') {
            output_redirects[output_count++] = temp;
            if (*(temp + 1) == '>') {
                temp += 2; // Skip both '>' characters
            } else {
                temp++;
            }
        } else {
            temp++;
        }
    }
    
    // Process all output redirections to check for file creation errors
    for (int i = 0; i < output_count; i++) {
        char* redir_pos = output_redirects[i];
        int append_mode = (*(redir_pos + 1) == '>') ? 1 : 0;
        char* file_start = redir_pos + 1;
        if (append_mode) file_start++; // Skip second '>' if append
        
        while (*file_start == ' ' || *file_start == '\t') {
            file_start++;
        }
        
        if (*file_start != '\0' && *file_start != '\n') {
            // Find end of filename
            char* file_end = file_start;
            while (*file_end != ' ' && *file_end != '\t' && *file_end != '\n' && *file_end != '\0' &&
                   *file_end != '<' && *file_end != '>') {
                file_end++;
            }
            
            // Test if file can be created/written
            char test_filename[256];
            int filename_len = file_end - file_start;
            strncpy(test_filename, file_start, filename_len);
            test_filename[filename_len] = '\0';
            
            int test_fd;
            if (append_mode) {
                test_fd = open(test_filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                test_fd = open(test_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            
            if (test_fd == -1) {
                printf("Unable to create file for writing\n");
                free(temp_copy);
                if (input_file) free(input_file);
                if (output_file) free(output_file);
                return;
            }
            close(test_fd);
            
            // If this is the last output redirection, save it
            if (i == output_count - 1) {
                output_file = malloc(filename_len + 1);
                strcpy(output_file, test_filename);
                is_append = append_mode;
            }
        }
    }
    
    free(temp_copy);
    
    // Find the LAST output redirection (both > and >>)
    char* last_output_redirect = NULL;
    char* last_append_redirect = NULL;
    temp = command_copy;
    
    while (*temp != '\0') {
        if (*temp == '>') {
            if (*(temp + 1) == '>') {
                last_append_redirect = temp;
                temp += 2; // Skip both '>' characters
            } else {
                last_output_redirect = temp;
                temp++  ;
            }
        } else {
            temp++;
        }
    }
    
    // Remove the redirection part from the command copy
    if (last_input_redirect != NULL) {
        *last_input_redirect = '\0';
    }
    
    // Process output redirection if found (only the last one)
    char* output_redirect_to_use = NULL;
    if (last_append_redirect != NULL) {
        output_redirect_to_use = last_append_redirect;
    } else if (last_output_redirect != NULL) {
        output_redirect_to_use = last_output_redirect;
    }
    
    if (output_redirect_to_use != NULL) {
        // Remove the redirection part from the command copy
        *output_redirect_to_use = '\0';
    }
    
    // Parse the command by reconstructing it without redirection parts
    char clean_command[512];
    char* src = command_copy;
    char* dst = clean_command;
    
    // Copy the command while skipping redirection parts
    while (*src != '\0') {
        if (*src == '<' || *src == '>') {
            // Skip past the redirection operator and its argument
            if (*src == '>' && *(src + 1) == '>') {
                src += 2; // Skip >>
            } else {
                src++; // Skip < or >
            }
            
            // Skip whitespace
            while (*src == ' ' || *src == '\t') src++;
            
            // Skip the filename
            while (*src != ' ' && *src != '\t' && *src != '\0' && 
                   *src != '<' && *src != '>') {
                src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    // Parse the clean command
    char* actual_command = strtok(clean_command, " \t\n");
    char* remaining_args = strtok(NULL, "\n");
    
    if (actual_command == NULL) {
        if (input_file) free(input_file);
        if (output_file) free(output_file);
        return;
    }
    
    // Handle built-in commands with redirection support
    if (strcmp(actual_command, "cd") == 0 || 
        strcmp(actual_command, "reveal") == 0 ||
        strcmp(actual_command, "hop") == 0 ||
        strcmp(actual_command, "log") == 0) {
        
        // Handle redirection for builtin commands
        int saved_stdout = -1, saved_stdin = -1;
        
        // Handle output redirection
        if (output_file != NULL) {
            saved_stdout = dup(STDOUT_FILENO);
            int fd;
            if (is_append) {
                fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd == -1) {
                printf("Unable to create file for writing\n");
                if (input_file) free(input_file);
                if (output_file) free(output_file);
                return;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
        // Handle input redirection
        if (input_file != NULL) {
            saved_stdin = dup(STDIN_FILENO);
            int fd = open(input_file, O_RDONLY);
            if (fd == -1) {
                printf("No such file or directory\n");
                if (input_file) free(input_file);
                if (output_file) free(output_file);
                return;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        // Execute the builtin command
        if (strcmp(actual_command, "cd") == 0 || strcmp(actual_command, "hop") == 0) {
            extern void hopping(char *currpath, char *args);
            hopping(currpath, remaining_args);
        } else if (strcmp(actual_command, "reveal") == 0) {
            extern void reveal(char *currpath, char *args, char *root);
            // Make a copy of the arguments to ensure clean string
            char* args_copy = NULL;
            if (remaining_args != NULL) {
                args_copy = strdup(remaining_args);
            }
            reveal(currpath, args_copy, homepath);
            if (args_copy) free(args_copy);
        } else if (strcmp(actual_command, "log") == 0) {
            // Handle log command if needed
        }
        
        // Restore stdout and stdin
        if (saved_stdout != -1) {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        if (saved_stdin != -1) {
            dup2(saved_stdin, STDIN_FILENO);
            close(saved_stdin);
        }
        
        if (input_file) free(input_file);
        if (output_file) free(output_file);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        if (is_background) {
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null != -1) {
                dup2(dev_null, STDIN_FILENO);
                close(dev_null);
            }
        }
        // Handle input redirection
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd == -1) {
                printf("No such file or directory\n");
                free(input_file);
                if (output_file) free(output_file);
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2 input failed");
                close(fd);
                free(input_file);
                if (output_file) free(output_file);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        
        // Handle output redirection
        if (output_file != NULL) {
            int fd;
            if (is_append) {
                fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            
            if (fd == -1) {
                printf("Unable to create file for writing\n");
                free(input_file);
                free(output_file);
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2 output failed");
                close(fd);
                free(input_file);
                free(output_file);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        
        // Free allocated memory
        if (input_file) free(input_file);
        if (output_file) free(output_file);
        
        // Prepare arguments for execvp
        char* argv[64];
        int argc = 0;
        argv[argc++] = actual_command;

        // Parse the remaining arguments
        if (remaining_args != NULL) {
            char* args_copy = strdup(remaining_args);
            char* token = strtok(args_copy, " \t");
            while (token != NULL && argc < 63) {
                argv[argc++] = strdup(token);  // Make sure to duplicate
                token = strtok(NULL, " \t");
            }
            free(args_copy);
        }

        argv[argc] = NULL;

        // Execute the external command
        execvp(actual_command, argv);

        perror("execvp");
        exit(EXIT_FAILURE);
        
    } else if (pid < 0) {
        perror("fork");
    } else {
        // --- Parent process ---
        if (input_file) free(input_file);
        if (output_file) free(output_file);
        
        // EDIT: Conditional waiting
        if (is_background) {
            add_job(pid, actual_command, true); // Print for background processes
        } else {
            // It's a foreground process, so we track its PID and name.
            foreground_pid = pid;
            strncpy((char*)foreground_command_name, actual_command, 255);

            int status;
            // Use waitpid with WUNTRACED to catch both termination and stops (Ctrl+Z).
            waitpid(pid, &status, WUNTRACED);

            // If the process wasn't stopped, it terminated, so we clear the foreground tracker.
            if (!WIFSTOPPED(status)) {
                foreground_pid = 0;
            }
        }
    }
}
