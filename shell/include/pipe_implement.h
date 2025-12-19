#ifndef PIPE_HI
#define PIPE_HI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "InputOutputRedir.h"

// Function to handle command piping with support for I/O redirection
// void execute_piped_commands(char* command_line);
void parse_command_args(char* command_segment, char* argv[]);
pid_t execute_pipe_segment(char* command_segment, int input_fd, int output_fd, bool is_first_command, bool is_background_pipeline);
void execute_piped_commands(char* command_line, bool is_background);

#endif // PIPE_H
