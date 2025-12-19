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

static char prev_cwd[256];
static char shell_home[256];
static bool prev_set = false;
static bool home_set = false;

void hopping(char *currpath, char *args) {

    // Set shell home on first call
    if (!home_set) {
        getcwd(shell_home, sizeof(shell_home));
        home_set = true;
    }

    getcwd(currpath, 256);

    // If no args -> go shell home
    if (args == NULL || strlen(args) == 0) {
        strcpy(prev_cwd, currpath);
        if (chdir(shell_home) != 0) {
            printf("No such directory!\n");
            return;
        }
        getcwd(currpath, 256);
        prev_set = true;
        return;
    }

    // Tokenize args
    char args_copy[256];
    strcpy(args_copy, args);
    char *token = strtok(args_copy, " \t\n");
    while (token != NULL) {
        if (strcmp(token, "~") == 0) {
            strcpy(prev_cwd, currpath);
            if (chdir(shell_home) != 0) {
                printf("No such directory!\n");
                return;
            }
            prev_set = true;
        }
        else if (strcmp(token, ".") == 0) {
            // stay in same dir - do nothing
        }
        else if (strcmp(token, "..") == 0) {
            strcpy(prev_cwd, currpath);
            if (chdir("..") != 0) {
                printf("No such directory!\n");
                return;
            }
            prev_set = true;
        }
        else if (strcmp(token, "-") == 0) {
            if (prev_set) {
                char temp[256];
                getcwd(temp, sizeof(temp));
                if (chdir(prev_cwd) != 0) {
                    printf("No such directory!\n");
                    return;
                }
                strcpy(prev_cwd, temp);
            }
            else{
                // Do nothing when no previous directory is tracked
            }
        }
        else {
            // change to given path
            strcpy(prev_cwd, currpath);
            if (chdir(token) != 0) {
                printf("No such directory!\n");
                return;
            } else {
                prev_set = true;
            }
        }
        getcwd(currpath, 256);
        token = strtok(NULL, " \t\n");
    }
}

char* get_prev_directory() {
    if (prev_set) {
        return prev_cwd;
    }
    return NULL;
}
