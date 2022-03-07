#ifndef SHELL_H
#define SHELL_H

#include "pcb.h"
#include "readyqueue.h"

int parseInput(char ui[], pcb_t *pcb, rq_t *rq);

#endif /* SHELL_H */