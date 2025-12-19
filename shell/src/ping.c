#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "ping.h"

void execute_ping(char* args) {
    if (args == NULL) {
        fprintf(stderr, "ping: missing arguments\nUsage: ping <pid> <signal_number>\n");
        return;
    }

    int pid, signal_number;
    if (sscanf(args, "%d %d", &pid, &signal_number) != 2) {
        fprintf(stderr, "ping: invalid arguments\nUsage: ping <pid> <signal_number>\n");
        return;
    }

    int actual_signal = signal_number % 32;

    // kill() returns 0 on success and -1 on error.
    if (kill(pid, actual_signal) == 0) {
        // Requirement 3: Success message
        printf("Sent signal %d to process with pid %d\n", actual_signal, pid);
    } else {
        // Requirement 2: Check if the error was "No such process"
        // perror("ping"); // You can uncomment this for more detailed errors
        printf("No such process found\n");
    }
}
