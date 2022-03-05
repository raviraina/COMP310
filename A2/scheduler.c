#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "readyqueue.h"
#include "shell.h"
#include "shellmemory.h"


// helper function to execute a line in the process
int execute_command(char *cmd, pcb_t *pcb, rq_t *rq) {
    // pass the stuff to parseInput function ????
    return parseInput(cmd, pcb, rq);
}


// Runs the processes in the ready queue according to FCFS scheduling policy
int FCFS_scheduler(rq_t *rq) {
    int err = 0;
    pcb_t *rq_head;

    // execute all processes in the ready queue one-by-one (FCFS)
    while ((rq_head = pop_rq_head(rq)) != NULL) {
        // execute the process line-by-line
        for (int i = 0; i < rq_head->size; i++) {
            err = execute_command(rq_head->pc->value, rq_head, rq);
            rq_head->pc ++;
        }

        // cleanup the current process
        mem_cleanup_script(rq_head);
    }
    return err;
}


// Runs the processes in the ready queue according to SJF scheduling policy
int SJF_scheduler() {
    return 0;
}


// Runs the processes in the ready queue according to RR scheduling policy
int RR_scheduler() {
    return 0;
}


// Runs the processes in the ready queue according to AGING scheduling policy
int AGING_scheduler() {
    return 0;
}