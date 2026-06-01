#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "command_processor.h"
#include "parser.h"
#include "hopping.h"
#include "history.h"
#include "reveal.h"
#include "BasicSysCall.h"
#include "InputOutputRedir.h"
#include "pipe_implement.h"
#include "process.h"
#include "job_control.h"
#include "ping.h"

/**
 * @brief process_single_command
 * 
 * This function processes a single command line string, tokenizes it, 
 * identifies if it should be run in the background, and handles 
 * the proper routing of execution based on the parsed tokens.
 * 
 * @param command_str The string containing the raw command.
 * @param currpath    The current working directory.
 * @param homepath    The user's home directory.
 */
void process_single_command(char* command_str, char* currpath, char* homepath) {
    // Trim leading/trailing whitespace
    char* end;
    while (isspace((unsigned char)*command_str)) command_str++;
    if (*command_str == 0) return;
    end = command_str + strlen(command_str) - 1;
    while (end > command_str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    // Detect if this command should run in the background
    bool is_background = false;
    int len = strlen(command_str);
    if (len > 0 && command_str[len - 1] == '&') {
        is_background = true;
        command_str[len - 1] = '\0'; // Remove the '&'
        // Re-trim whitespace in case user typed "cmd & "
        end = command_str + strlen(command_str) - 1;
        while (end > command_str && isspace((unsigned char)*end)) end--;
        end[1] = '\0';
    }

    char original_command[4096];
    strcpy(original_command, command_str);
    char parsing_copy[4096];
    strcpy(parsing_copy, command_str);

    char* first_command = strtok(parsing_copy, " \t\n");
    if (first_command == NULL) return;
    
    // Get the rest of the arguments by finding where first_command ends in original_command
    char* rest = NULL;
    char* space_pos = strchr(original_command, ' ');
    if (space_pos != NULL) {
        rest = space_pos + 1;
        // Skip leading whitespace
        while (*rest && (*rest == ' ' || *rest == '\t')) {
            rest++;
        }
        if (*rest == '\0') rest = NULL; // No arguments after command
    }

    // Check for REDIRECTION and PIPES first, as they can contain builtin commands
    if (strchr(original_command, '|') != NULL) {
        execute_piped_commands(original_command, is_background);
    } else if (strchr(original_command, '>') != NULL || strchr(original_command, '<') != NULL) {
        input_output_redirection2(original_command, currpath, homepath);
    }
    // Then check for BUILT-IN commands. They NEVER run in the background.
    else if (strcmp(first_command, "hop") == 0) {
        hopping(currpath, rest);
    } else if (strcmp(first_command, "reveal") == 0) {
        reveal(currpath, rest, homepath);
    } else if (strcmp(first_command, "log") == 0) {
        log_command(rest, currpath, homepath);
    }
    else if (strcmp(first_command, "activities") == 0) {
        execute_activities();
    }
    else if (strcmp(first_command, "ping") == 0) {
        execute_ping(rest);
    }
    else if (strcmp(first_command, "fg") == 0) {
        execute_fg(rest);
    }
    else if (strcmp(first_command, "bg") == 0) {
        execute_bg(rest);
    }
    // If it's not a built-in, it's an EXTERNAL command.
    else {
        execute_command(first_command, rest, is_background);
    }
}
