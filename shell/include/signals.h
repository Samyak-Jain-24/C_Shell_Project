#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <unistd.h>

// Signal handler function prototypes
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void setup_signal_handlers();

#endif // SIGNALS_H
