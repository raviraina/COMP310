#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include "pcb.h"

typedef struct ready_queue {
    struct pcb *head;
    struct pcb *tail;
    struct pcb *curr;
} rq_t;

rq_t *init_rq();
void add_pcb(rq_t *rq, pcb_t *pcb);
pcb_t *pop_rq_head(rq_t *rq);

#endif /* READY_QUEUE_H */