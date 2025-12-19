#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <ctype.h>
#include "parser.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "reveal.h"
#include "hopping.h"

int cmpfunc(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void reveal(char *currpath, char *args, char *root)
{
    int show_hidden = 0;
    int line_format = 0;
    char path[256];
    strcpy(path, currpath); // default to current path
    int too_many_args = 0;
    int path_count = 0;

    // Parse args
    if (args != NULL)
    {
        char args_copy[256];
        strcpy(args_copy, args);
        char *token = strtok(args_copy, " \t");
        while (token != NULL)
        {
            if (strcmp(token, "-") == 0)
            {
                // Handle previous directory case first (before flag processing)
                path_count++;
                if (path_count > 1) {
                    too_many_args = 1;
                    break;
                }
                char* prev_dir = get_prev_directory();
                if (prev_dir != NULL) {
                    strcpy(path, prev_dir);
                } else {
                    printf("No such directory!\n");
                    return;
                }
            }
            else if (token[0] == '-')
            {
                // Flags
                for (int j = 1; token[j] != '\0'; j++)
                {
                    if (token[j] == 'a')
                        show_hidden = 1;
                    else if (token[j] == 'l')
                        line_format = 1;
                }
            }
            else
            {
                // Path argument
                path_count++;
                if (path_count > 1) {
                    too_many_args = 1;
                    break;
                }
                
                if (strcmp(token, "~") == 0)
                {
                    strcpy(path, root);
                }
                else if (strcmp(token, "-") == 0)
                {
                    // Handle previous directory
                    char* prev_dir = get_prev_directory();
                    if (prev_dir != NULL) {
                        strcpy(path, prev_dir);
                    } else {
                        printf("No such directory!\n");
                        return;
                    }
                }
                else
                {
                    // Handle relative and absolute paths
                    if (token[0] == '/') {
                        // Absolute path
                        strcpy(path, token);
                    } else {
                        // Relative path - construct full path
                        snprintf(path, sizeof(path), "%s/%s", currpath, token);
                    }
                }
            }
            token = strtok(NULL, " \t");
        }
    }

    if (too_many_args) {
        printf("reveal: Invalid Syntax!\n");
        return;
    }

    DIR *dp = opendir(path);
    if (dp == NULL)
    {
        printf("No such directory!\n");
        return;
    }

    struct dirent *entry;
    char *files[10000];
    int count = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        // Skip hidden files unless -a flag is set
        if (!show_hidden && entry->d_name[0] == '.')
        {
            continue;
        }

        // Always skip .shell_history file regardless of flags
        if (strcmp(entry->d_name, ".shell_history") == 0) {
            continue;
        }

        files[count++] = strdup(entry->d_name);
    }
    closedir(dp);

    qsort(files, count, sizeof(char *), cmpfunc);

    for (int i = 0; i < count; i++)
    {
        if (line_format)
        {
            printf("%s\n", files[i]);
        }
        else
        {
            printf("%s", files[i]);
            if (i != count - 1)
                printf(" ");
        }
        free(files[i]);
    }
    if (!line_format && count > 0)
        printf("\n");
}
