#ifndef a1_H
#define a1_H

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

void execute_command(char* command, char* args,bool is_background);

#endif
