#ifndef PCB_H
#define PCB_H

#include "shellmemory.h"

struct pcb {
    int pid; // unique pid of process.
    struct memory_struct *base; // pointer to beginning of memory block where the script is stored.
    int size; // size of the memory block i.e., number of lines of code in the script.
    struct memory_struct *pc; // pointer to the current line of code being executed -- program counter.
};

#endif /* PCB_H */