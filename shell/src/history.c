#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>
#include<ctype.h>
#include "parser.h"
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include "hopping.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hopping.h"
#include "history.h"
#include "BasicSysCall.h"
#include "reveal.h"

/* 
 * =========================================================================
 * HISTORY MANAGEMENT
 * -------------------------------------------------------------------------
 * This module is responsible for loading, managing, and saving the command
 * history across shell sessions. 
 * =========================================================================
 */

// Definition of global variable for storing command history state
CommandHistory history;

void init_history() {
    FILE *file = fopen(LOG_FILE, "r");
    if (!file) {
        history.count = 0;
        return;
    }

    history.count = 0;
    //   while (history.count < MAX_LOG_SIZE && fgets(history.commands[history.count], 256, file)) {
    //     history.commands[history.count][strcspn(history.commands[history.count], "\n")] = 0;
    //     history.count++;
    // }
    fclose(file);
}

void save_history() {
    FILE *file = fopen(LOG_FILE, "w");
    if (!file) return;

    for (int i = 0; i < history.count; i++) {
        fprintf(file, "%s\n", history.commands[i]);
    }
    fclose(file);
}

void add_to_history(const char* command) {
    if (strlen(command) == 0 || strncmp(command, "log", 3) == 0) {
        return;
    }

    if (history.count > 0 && strcmp(history.commands[history.count-1], command) == 0) {
        return;
    }

    if (history.count == MAX_LOG_SIZE) {
        for (int i = 0; i < MAX_LOG_SIZE - 1; i++) {
            strcpy(history.commands[i], history.commands[i+1]);
        }
        history.count--;
    }

    strcpy(history.commands[history.count], command);
    history.count++;
    save_history();
}
void log_command(char* args, char* currpath, char* cwd) {
    if (args == NULL || strlen(args) == 0) {
        for (int i = 0; i < history.count; i++) {
            // Print with a newline since we now store them without one
            printf("%s\n", history.commands[i]);
        }
    } else {
        char* subcmd = strtok(args, " \t\n");
        if (strcmp(subcmd, "purge") == 0) {
            history.count = 0;
            save_history();
        } else if (strcmp(subcmd, "execute") == 0) {
            char* index_str = strtok(NULL, " \t\n");
            if (index_str) {
                int index = atoi(index_str);
                if (index > 0 && index <= history.count) {
                    // History is 1-based for users, but 0-based in our array
                    // Also, we want the Nth most recent command.
                    int real_index = history.count - index;
                    
                    // EDIT: Start of new logic to handle '&' from history
                    char command_to_run[256];
                    strcpy(command_to_run, history.commands[real_index]);
                    
                    bool is_background_log = false;
                    int len = strlen(command_to_run);
                    if (len > 0 && command_to_run[len - 1] == '&') {
                        is_background_log = true;
                        command_to_run[len - 1] = '\0'; // Remove the '&'
                        // Trim any space before the '&'
                        char* end = command_to_run + strlen(command_to_run) - 1;
                        while (end > command_to_run && isspace((unsigned char)*end)) {
                            *end-- = '\0';
                        }
                    }
                    // EDIT: End of new logic

                    char* cmd = strtok(command_to_run, " \t\n");
                    char* rest = strtok(NULL, "\n");

                    // Check for built-ins first
                    if (strcmp(cmd, "hop") == 0) {
                        hopping(currpath, rest);
                    } else if (strcmp(cmd, "reveal") == 0) {
                        char copy_currpath[256];
                        strcpy(copy_currpath, currpath);
                        reveal(currpath, rest, cwd);
                        strcpy(currpath, copy_currpath);
                    } else {
                        // EDIT: Pass the new boolean flag to the function
                        execute_command(cmd, rest, is_background_log);
                    }
                } else {
                    printf("Invalid index\n");
                }
            } else {
                printf("Missing index\n");
            }
        } else {
            printf("Invalid log subcommand\n");
        }
    }
}
