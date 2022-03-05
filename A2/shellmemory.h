#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#include "pcb.h"

struct memory_struct{
	char *var;
	char *value;
};

void mem_init();
char *mem_get_value(char *var, pcb_t *pcb);
void mem_set_value(char *var, char *value, pcb_t *pcb);
int check_mem_value_exists(char *var_in, pcb_t *pcb);

int mem_load_script_line(int pid, int line_number, char *script_line, struct memory_struct *mem);
int mem_load_script(FILE *script, pcb_t *pcb);
int mem_cleanup_script(pcb_t *pcb);

#endif /* SHELLMEMORY_H */
