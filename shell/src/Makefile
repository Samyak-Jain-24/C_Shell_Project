# Compiler and flags
CC = gcc
CFLAGS = -std=c99 \
         -D_POSIX_C_SOURCE=200809L \
         -D_XOPEN_SOURCE=700 \
         -Wall -Wextra -Werror \
         -Wno-unused-parameter \
         -fno-asm \
         -I../include

# Project structure  
TARGET = ../shell.out
SRC = main_new.c BasicSysCall.c history.c hopping.c parser.c reveal.c InputOutputRedir2.c pipe_implement.c jobs.c signals.c process.c job_control.c ping.c path_utils.c command_processor.c
OBJ = $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Linking final binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compiling .c -> .o  
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
