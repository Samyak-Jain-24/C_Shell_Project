#ifndef HOPPING_H
#define HOPPING_H

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

void hopping(char *currpath, char *args);
char* get_prev_directory();

#endif
