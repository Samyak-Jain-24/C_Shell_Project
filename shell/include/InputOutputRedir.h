#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>
#include<ctype.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>

#ifndef IO_H
#define IO_H

void input_output_redirection2(char* command, char* currpath, char* homepath);

#endif