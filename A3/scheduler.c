#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "pcb.h"
#include "readyqueue.h"
#include "shell.h"
#include "shellmemory.h"


// helper function to increment a proram's PC -- sets PC to NULL if it has already reached the end of the program
// returns 0 if PC incremented successfully, 1 if page fault
int increment_pc(pcb_t *pcb) {
    // check whether the current PC is the last command in the current frame
    // if so, get the next frame and set PC to the first command of that frame
    // if not, simply do PC++
    struct memory_struct *frame_last_entry = mem_get_entry(pcb->page_table[pcb->curr_page], FRAME_SIZE - 1);
    if (pcb->pc < frame_last_entry) {
        pcb->pc++;
        return 0;
    } else {
        // check whether we have reached the end of the script
        if (pcb->curr_page == pcb->num_pages - 1) {
            pcb->pc = NULL;
            return 0;
        }

        // check whether the next page of the script is available in memory. If so, increment PC 
        if (pcb->page_table[pcb->curr_page + 1] != -1) {
            pcb->curr_page++;
            pcb->pc = mem_get_entry(pcb->page_table[pcb->curr_page], 0);
            return 0;
        } 

        // if none of the previous two are the case, throw a page fault
        return -1;
    }
}


int handle_page_fault(pcb_t *pcb, rq_t *rq) {
    printf("<<<HANDLING PAGE FAULT>>>\n");

    // handle page fault
    char *page[FRAME_SIZE];
    int err = 0;

    // load the next page (pcb->curr_page + 1) of process as char *page[FRAME_SIZE]
    // IGNORE: note that it automatically increments the pcb->curr_page [NOT FOR NOW]
    load_page(pcb, pcb->curr_page + 1, page);

    // store the page into a frame
    err = mem_load_frame(pcb, page, pcb->curr_page + 1, rq);

    // increment the PC of the program accordingly
    assert(increment_pc(pcb) == 0);
    
    // add pcb to the tail of the ready queue
    add_rq_tail(rq, pcb);

    return err;
}



// helper function to execute a line in the process
// returns error code, -9 if page fault
int execute_command(pcb_t *pcb, rq_t *rq) {
    // pass the stuff to parseInput function
    int err = parseInput(pcb->pc->value, pcb, rq);

    // increment the process's PC
    if (increment_pc(pcb) != 0) {
        // send page fault signal to scheduler
        return -9;
    }

    // return the error code
    return err;
}

// helper function to compare two PCBs according to their size for SJF scheduler
int compare_pcb_size(const void *a, const void *b) {
    pcb_t *pcb1 = *(pcb_t **) a;
    pcb_t *pcb2 = *(pcb_t **) b;

    return pcb1->size - pcb2->size;
}


// helper function to sort the ready queue based on their size (i.e., script length)
void sort_rq_size(rq_t *rq) {
    pcb_t *rq_head;
    pcb_t **pcb_array = calloc(rq->size, sizeof(pcb_t *));
    int i = 0;
    
    // sort the ready queue in ascending order of the process' size
    while((rq_head = pop_rq_head(rq)) != NULL) {
        pcb_array[i++] = rq_head;
    }
    
    // quick sort the array of PCBs based on their sizes
    qsort(pcb_array, i, sizeof(pcb_t *), compare_pcb_size);

    // push the PCBs back to the ready queue in the sorted order
    for (int j = 0; j < i; j++) {
        add_rq_tail(rq, pcb_array[j]);
    }

    // free memory
    free(pcb_array);
}


// Runs the processes in the ready queue according to FCFS scheduling policy
int FCFS_scheduler(rq_t *rq) {
    int pf, err = 0;
    pcb_t *rq_head;
    // execute all processes in the ready queue one-by-one (FCFS)
    while ((rq_head = pop_rq_head(rq)) != NULL) {
        while(rq_head->pc != NULL) {
            err = execute_command(rq_head, rq);
            pf = 0;
            if (err == -9) {
                // handle page fault and continue with the program execution
                handle_page_fault(rq_head, rq);
                pf = 1;
                break;
            }
        }
        
        if(pf) {
            continue;
        }

        // cleanup the current process
        mem_cleanup_script(rq_head);

    }
    return err;
}


// Runs the processes in the ready queue according to SJF scheduling policy
int SJF_scheduler(rq_t *rq) {
    // sort the ready queue in ascending order of the process' size
    sort_rq_size(rq);

    // execute all processes like FCFS
    return FCFS_scheduler(rq);
}


// Runs the processes in the ready queue according to RR scheduling policy
// Note: To make things simpler, in this assignment one-liners are considered as a single instruction.
int RR_scheduler(rq_t *rq) {
    int pf, err = 0;
    pcb_t *rq_head;
    
    while ((rq_head = pop_rq_head(rq)) != NULL) {   
        // execute two instructions of the process
        for (int i =  0; (i < 2) && (rq_head->pc != NULL); i++) {
            err = execute_command(rq_head, rq);
            if (err == -9) {
                // handle page fault and continue with the program execution
                handle_page_fault(rq_head, rq);
                pf = 1;
                break;
            }

        }

        if (rq_head->pc != NULL) {
            // add the process back to the ready queue (waiting state) if it hasn't reached its end yet
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
    int err, min_jls;
    pcb_t *rq_head, *curr, *min_jls_pcb;
    
    // sort the ready queue in ascending order of the process' size (initially, JLS = script length)
    sort_rq_size(rq);

    // start executing processes
    while((rq_head = pop_rq_head(rq)) != NULL) {
        // execute the head process for one instruction
        err = execute_command(rq_head, rq);

        // Process AGING: update the JLS of processes (except the head process) in the ready queue
        curr = peek_rq_head(rq);
        while (curr != NULL) {
            if(curr->jls > 0) curr->jls--;
            curr = curr->next;
        }
        
        // find the process (other than the head process) with the minimum JLS
        min_jls = INT_MAX;
        curr = peek_rq_head(rq);
        while (curr != NULL) {
            if (curr->jls < min_jls) {
                min_jls = curr->jls;
                min_jls_pcb = curr;
            }
            curr = curr->next;
        }

        // if the head process has finished execution, cleanup the memory.
        if (rq_head->pc == NULL) {
            mem_cleanup_script(rq_head);
            min_jls_pcb = remove_rq_pcb(rq, min_jls_pcb);
            add_rq_head(rq, min_jls_pcb);
        }
        // otherwise, insert the processeses back into the queue according to their JLS
        else {
            if (min_jls < rq_head->jls) {
                add_rq_tail(rq, rq_head);
                min_jls_pcb = remove_rq_pcb(rq, min_jls_pcb);
                add_rq_head(rq, min_jls_pcb);
            } else {
                add_rq_head(rq, rq_head);
            }
        }
    }
    return err;
}