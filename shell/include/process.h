#ifndef PROCESS_H
#define PROCESS_H

#include <unistd.h>
#include "jobs.h"

// Process management function prototypes
const char* get_process_state(pid_t pid);
void execute_activities();
int compare_jobs(const void* a, const void* b);

#endif // PROCESS_H
