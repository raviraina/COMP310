#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "pcb.h"
#include "readyqueue.h"

int interpreter(char* command_args[], int args_size, pcb_t *pcb, rq_t *rq);
int help();

#endif /* INTERPRETER_H */