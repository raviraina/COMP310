#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#include "pcb.h"

struct memory_struct{
	char *var;
	char *value;
};

extern const int FRAME_SIZE;
extern const int VAR_MEM_SIZE;
extern const int FREE_LIST_SIZE;

void mem_init();
struct memory_struct *mem_get_entry(int frame_number, int offset);

char *mem_get_value(char *var, pcb_t *pcb);
void mem_set_value(char *var, char *value, pcb_t *pcb);
int check_mem_value_exists(char *var_in, pcb_t *pcb);

int mem_load_script_line(int pid, int line_number, char *script_line, struct memory_struct *mem);
int mem_load_script(FILE *script, pcb_t *pcb);
int mem_cleanup_script(pcb_t *pcb);

#endif /* SHELLMEMORY_H */
