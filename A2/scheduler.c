#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "readyqueue.h"
#include "shell.h"
#include "shellmemory.h"


// helper function to execute a line in the process
int execute_command(pcb_t *pcb, rq_t *rq) {
    // pass the stuff to parseInput function ????
    return parseInput(pcb->pc->value, pcb, rq);
}

// helper function to compare two PCBs according to their size for SJF scheduler
int compare_pcb_size(const void *a, const void *b) {
    pcb_t *pcb1 = *(pcb_t **) a;
    pcb_t *pcb2 = *(pcb_t **) b;

    return pcb1->size - pcb2->size;
}


// Runs the processes in the ready queue according to FCFS scheduling policy
int FCFS_scheduler(rq_t *rq) {
    int err = 0;
    pcb_t *rq_head;

    // execute all processes in the ready queue one-by-one (FCFS)
    while ((rq_head = pop_rq_head(rq)) != NULL) {
        // printf("Script size = %d\n", rq_head->size);
        // execute the process line-by-line
        for (int i = 0; i < rq_head->size; i++) {
            err = execute_command(rq_head, rq);
            rq_head->pc ++;
        }

        // cleanup the current process
        mem_cleanup_script(rq_head);
    }
    return err;
}


// Runs the processes in the ready queue according to SJF scheduling policy
int SJF_scheduler(rq_t *rq) {
    pcb_t *rq_head;
    pcb_t **pcb_array = calloc(rq->size, sizeof(pcb_t *));
    int i = 0;
    
    // sort the ready queue in ascending order of the process' size
    while((rq_head = pop_rq_head(rq)) != NULL) {
        pcb_array[i++] = rq_head;
    }
    
    qsort(pcb_array, i, sizeof(pcb_t *), compare_pcb_size);



    // add the sorted PCBs back to the ready queue
    i--;
    while(i-- > 0) {
        add_rq_tail(rq, pcb_array[i]);
    }

    // for (int j = 0; j < i; j++) {
        
    // }
    // free(pcb_array);

    // execute all processes like FCFS
    return FCFS_scheduler(rq);
}


// Runs the processes in the ready queue according to RR scheduling policy
// Note: To make things simpler, in this assignment one-liners are considered as a single instruction.
int RR_scheduler(rq_t *rq) {
    int err = 0;
    pcb_t *rq_head;
    
    while ((rq_head = pop_rq_head(rq)) != NULL) {   
        // execute two instructions of the process
        for (int i =  0; (i < 2) && (rq_head->pc <= (rq_head->base + rq_head->size - 1)); i++) {
            err = execute_command(rq_head, rq);
            rq_head->pc ++;
        }

        if (rq_head->pc < (rq_head->base + rq_head->size - 1)) {
            // add the process back to the ready queue (waiting state)
            add_rq_tail(rq, rq_head);
        } else {
            // cleanup the current process
            mem_cleanup_script(rq_head);
        }
    }
    return err;
}


// Runs the processes in the ready queue according to AGING scheduling policy
int AGING_scheduler(rq_t *rq) {
    return 0;
}