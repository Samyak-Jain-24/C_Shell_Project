// parser.h
#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

// Forward declarations
bool parse_shell_cmd(const char *input, size_t *pos, size_t len);
bool parse_cmd_group(const char *input, size_t *pos, size_t len);
bool parse_atomic(const char *input, size_t *pos, size_t len);
bool parse_input(const char *input, size_t *pos, size_t len);
bool parse_output(const char *input, size_t *pos, size_t len);
bool parse_name(const char *input, size_t *pos, size_t len);
bool is_valid_syntax(const char *input);
void skip_whitespace(const char *input, size_t *pos, size_t len);

#endif
