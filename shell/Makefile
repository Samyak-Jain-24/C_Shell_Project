# Shell Assignment Makefile
# Compiles to shell.out in the Shell directory as required by assignment

CC = gcc
CFLAGS = -std=c99 \
         -D_POSIX_C_SOURCE=200809L \
         -D_XOPEN_SOURCE=700 \
         -Wall -Wextra -Werror \
         -Wno-unused-parameter \
         -fno-asm

# Include directory
INCLUDE_DIR = include
SRC_DIR = src

# Source files
SOURCES = $(SRC_DIR)/main_new.c \
          $(SRC_DIR)/BasicSysCall.c \
          $(SRC_DIR)/history.c \
          $(SRC_DIR)/hopping.c \
          $(SRC_DIR)/parser.c \
          $(SRC_DIR)/reveal.c \
          $(SRC_DIR)/InputOutputRedir2.c \
          $(SRC_DIR)/pipe_implement.c \
          $(SRC_DIR)/jobs.c \
          $(SRC_DIR)/signals.c \
          $(SRC_DIR)/process.c \
          $(SRC_DIR)/job_control.c \
          $(SRC_DIR)/ping.c \
          $(SRC_DIR)/path_utils.c \
          $(SRC_DIR)/command_processor.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Target
TARGET = shell.out

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
