#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include "pcb.h"

typedef struct ready_queue {
    pcb_t *head;
    pcb_t *tail;
    pcb_t *curr;
} rq_t;

rq_t *init_rq();
void add_rq_tail(rq_t *rq, pcb_t *pcb);
pcb_t *pop_rq_head(rq_t *rq);

#endif /* READY_QUEUE_H */