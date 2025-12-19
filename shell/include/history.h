#ifndef HISTORY_H
#define HISTORY_H

#define MAX_LOG_SIZE 15
#define LOG_FILE ".shell_history"

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

typedef struct {
    char commands[MAX_LOG_SIZE][256];
    int count;
} CommandHistory;

extern CommandHistory history;  // Declaration of global variable

void init_history();
void save_history();
void add_to_history(const char* command);
void log_command(char* args,char* currpath,char* cwd);

#endif