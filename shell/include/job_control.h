#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include "jobs.h"

// Job control function prototypes
BackgroundJob* find_job_by_id(int job_id);
BackgroundJob* find_most_recent_job();
void execute_fg(char* args);
void execute_bg(char* args);

#endif // JOB_CONTROL_H
