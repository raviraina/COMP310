#ifndef PCB_H
#define PCB_H

typedef struct pcb {
    int pid; // unique pid of process.
    struct memory_struct *base; // pointer to beginning of memory block where the script is stored.
    int size; // size of the memory block i.e., number of lines of code in the script.
    struct memory_struct *pc; // pointer to the current line of code being executed -- program counter.
    struct pcb *next; // pointer to the next pcb in the ready queue.
    int jls; // job length score -- computed by the SJF-AGING scheduler
    int num_pages; // number of pages in the script
    int* page_table; // page table to keep track of pages of the process. Will be initialized to point to appropriate pages when the process is loaded into memory.
} pcb_t;

#endif /* PCB_H */