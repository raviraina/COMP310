#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "readyqueue.h"


// initializes a ready queue and returns a pointer to it
rq_t *init_rq() {
    rq_t *rq = malloc(sizeof(rq_t));
    rq->head = NULL;
    rq->tail = NULL;
    rq->size = 0;
    return rq;
}


// adds a PCB to the ready queue
void add_rq_tail(rq_t *rq, pcb_t *pcb) {
    if (rq->head == NULL) {
        rq->head = pcb;
        rq->tail = pcb;
    } else {
        rq->tail->next = pcb;
        rq->tail = pcb;
    }
    rq->size++;
}


// pops the head of ready queue and returns a pointer to it
pcb_t *pop_rq_head(rq_t *rq) {
    if (rq->head == NULL) return NULL;
    pcb_t *head = rq->head;
    rq->head = rq->head->next;
    rq->size--;
    head->next = NULL;
    return head;
} 


// removes the PCB with given PID from the ready queue
pcb_t *remove_pcb(rq_t *rq, pcb_t *pcb) {
    if (rq->head == NULL) return NULL;
    if (rq->head == pcb) {
        rq->head = rq->head->next;
        rq->size--;
        return pcb;
    }
    pcb_t *curr = rq->head;
    while (curr->next != NULL) {
        if (curr->next == pcb) {
            curr->next = curr->next->next;
            rq->size--;
            return pcb;
        }
        curr = curr->next;
    }
    return NULL;
}