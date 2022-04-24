#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "readyqueue.h"

int FCFS_scheduler(rq_t *rq);
int SJF_scheduler(rq_t *rq);
int RR_scheduler(rq_t *rq);
int AGING_scheduler(rq_t *rq);

#endif /* SCHEDULER_H */