#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "readyqueue.h"


// initializes a ready queue and returns a pointer to it
rq_t *init_rq() {
    rq_t *rq = malloc(sizeof(rq_t));
    rq->head = NULL;
    rq->tail = NULL;
    rq->curr = rq->head;
    return rq;
}


// adds a PCB to the ready queue
void add_pcb(rq_t *rq, pcb_t *pcb) {
    if (rq->head == NULL) {
        rq->head = pcb;
        rq->tail = pcb;
    } else {
        rq->tail->next = pcb;
        rq->tail = pcb;
    }
}


// pops the head of ready queue and returns a pointer to it
pcb_t *pop_rq_head(rq_t *rq) {
    if (rq->head == NULL) return NULL;
    pcb_t *head = rq->head;
    rq->head = rq->head->next;
    return head;
} 