# Custom C Shell Project

A fully functional, robust UNIX shell implemented entirely in C. This project demonstrates core operating system concepts including process management, inter-process communication, signal handling, and file descriptor manipulation.

## 🚀 Features

This custom shell supports a wide variety of standard UNIX shell features, built from scratch:

### 1. Core Command Execution
- **System Commands:** Execution of standard Linux commands (e.g., `echo`, `cat`, `grep`) using `fork()` and `execvp()`.
- **Foreground & Background Processing:** Commands can be run in the background by appending `&` to the command. The shell continues to accept input while the background process executes.

### 2. Built-in Commands
- **`hop` (cd):** Navigates the filesystem. Supports relative/absolute paths, `.` (current directory), `..` (parent directory), `~` (home directory), and `-` (previous directory).
- **`reveal` (ls):** Lists files and directories. Supports `-l` (long format) and `-a` (show hidden files) flags, displaying permissions, ownership, and timestamps.
- **`history`:** Maintains a persistent command history across shell sessions. Can display the most recent commands executed.
- **`jobs`:** Lists all currently running background jobs with their job ID, state (Running/Stopped), and PID.

### 3. Process Management & Job Control
- **`ping`:** A custom utility to send signals to running processes by PID.
- **`fg` & `bg`:** Brings a stopped or background job into the foreground (`fg`), or resumes a stopped job in the background (`bg`).
- **Signal Handling:** 
  - `Ctrl+C` (SIGINT): Interrupts any currently running foreground process without killing the shell itself.
  - `Ctrl+Z` (SIGTSTP): Stops (suspends) the current foreground process and pushes it to the background jobs list.
  - `Ctrl+D` (EOF): Gracefully exits the shell and kills all background processes.

### 4. Input/Output Redirection & Piping
- **Redirection:** Supports standard input (`<`), standard output (`>`), and append output (`>>`) redirection. 
- **Piping:** Supports chaining multiple commands together using the pipe operator (`|`). Data flows seamlessly from the standard output of the left command to the standard input of the right command.

## 📁 Project Structure

```text
C_Shell_Project/
├── shell/
│   ├── include/          # Header files for all shell modules
│   │   ├── history.h
│   │   ├── job_control.h
│   │   ├── pipe_implement.h
│   │   └── ...
│   ├── src/              # Source code implementations
│   │   ├── BasicSysCall.c
│   │   ├── parser.c
│   │   ├── signals.c
│   │   ├── InputOutputRedir2.c
│   │   └── ...
│   └── Makefile          # Build configuration
└── README.md
```

## 🛠️ Build and Run Instructions

### Prerequisites
- GCC compiler
- Make utility
- Linux environment (or WSL on Windows)

### Compilation
Navigate to the `shell/` directory and run `make`:
```bash
cd shell
make
```

### Execution
Run the compiled executable to start the shell:
```bash
./a.out
```

## 🧠 Technical Highlights
- **Modular Design:** The codebase is heavily modularized, separating concerns like parsing (`parser.c`), piping (`pipe_implement.c`), and signal handling (`signals.c`).
- **Persistent State:** Uses a hidden `.shell_history` file to persist user command history securely between independent sessions.
- **Robust Parsing:** Implements a custom tokenizer that correctly handles arbitrary spacing, tabs, and complex command chains involving both pipes and redirection simultaneously.

---
*Developed as a comprehensive exercise in Operating Systems concepts.*
