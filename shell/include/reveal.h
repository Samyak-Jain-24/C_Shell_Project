#ifndef A2_H
#define A2_H

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

int cmpfunc(const void *a, const void *b);
void reveal(char *currpath, char *args, char* root);

#endif