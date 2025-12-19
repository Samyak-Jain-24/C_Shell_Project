// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "parser.h"

void skip_whitespace(const char *input, size_t *pos, size_t len) {
    while (*pos < len && isspace(input[*pos])) {
        (*pos)++;
    }
}

bool parse_name(const char *input, size_t *pos, size_t len) {
    skip_whitespace(input, pos, len);
    size_t start = *pos;
    
    while (*pos < len) {
        char c = input[*pos];
        if (c == '&' || c == '>' || c == '<' || c == ';' || c == '|' || isspace(c)) {
            break;
        }
        (*pos)++;
    }
    
    return *pos > start;
}

bool parse_input(const char *input, size_t *pos, size_t len) {
    skip_whitespace(input, pos, len);
    if (*pos >= len || input[*pos] != '<') {
        return false;
    }
    (*pos)++;
    
    skip_whitespace(input, pos, len);
    return parse_name(input, pos, len);
}

bool parse_output(const char *input, size_t *pos, size_t len) {
    skip_whitespace(input, pos, len);
    if (*pos >= len || input[*pos] != '>') {
        return false;
    }
    (*pos)++;
    
    if (*pos < len && input[*pos] == '>') {
        (*pos)++;
    }
    
    skip_whitespace(input, pos, len);
    return parse_name(input, pos, len);
}

bool parse_atomic(const char *input, size_t *pos, size_t len) {
    if (!parse_name(input, pos, len)) {
        return false;
    }
    
    while (true) {
        skip_whitespace(input, pos, len);
        if (*pos >= len) break;
        
        char c = input[*pos];
        if (c == '<') {
            if (!parse_input(input, pos, len)) return false;
        } 
        else if (c == '>') {
            if (!parse_output(input, pos, len)) return false;
        } 
        else if (c == '|' || c == '&' || c == ';') {
            break;
        } 
        else {
            if (!parse_name(input, pos, len)) return false;
        }
    }
    
    return true;
}

bool parse_cmd_group(const char *input, size_t *pos, size_t len) {
    if (!parse_atomic(input, pos, len)) {
        return false;
    }
    
    while (true) {
        skip_whitespace(input, pos, len);
        if (*pos >= len || input[*pos] != '|') {
            break;
        }
        (*pos)++;
        
        if (!parse_atomic(input, pos, len)) {
            return false;
        }
    }
    
    return true;
}

bool parse_shell_cmd(const char *input, size_t *pos, size_t len) {
    if (!parse_cmd_group(input, pos, len)) {
        return false;
    }
    
    while (true) {
        skip_whitespace(input, pos, len);
        if (*pos >= len) break;
        
        if (input[*pos] == '&') {
            (*pos)++;
            skip_whitespace(input, pos, len);
            if(*pos>=len) return true;
            
            if (!parse_cmd_group(input, pos, len)) {
                return false;
            }
        } 
        else if (input[*pos] == ';'){
            (*pos)++;
            skip_whitespace(input, pos, len);
            if (!parse_cmd_group(input, pos, len)) {
                return false;
            }
        }
        else {
            break;
        }
    }
    
    skip_whitespace(input, pos, len);
    if (*pos < len && input[*pos] == '&') {
        (*pos)++;
    }
    
    skip_whitespace(input, pos, len);
    return *pos == len;
}

bool is_valid_syntax(const char *input) {
    if (input == NULL || strlen(input) == 0) {
        return false;
    }
    
    size_t pos = 0;
    size_t len = strlen(input);
    return parse_shell_cmd(input, &pos, len);
}
