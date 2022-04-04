#ifndef FRAME_H
#define FRAME_H

#include "pcb.h"

typedef struct frame {
    int is_available; // whether the frame is currently occupied
    int age; // the age of the frame (used by LRU page replacement policy)
    pcb_t *pcb; // pointer to the PCB of the process that currently occupies the frame; NULL if not occupied
} frame_t;

#endif /* FRAME_H */