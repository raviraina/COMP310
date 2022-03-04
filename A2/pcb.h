#ifndef PCB_H
#define PCB_H

#include "shellmemory.h"

typedef struct pcb {
    int pid; // unique pid of process.
    struct memory_struct *base; // pointer to beginning of memory block where the script is stored.
    int size; // size of the memory block i.e., number of lines of code in the script.
    struct memory_struct *pc; // pointer to the current line of code being executed -- program counter.
    struct pcb *next; // pointer to the next pcb in the ready queue.
} pcb_t;

#endif /* PCB_H */